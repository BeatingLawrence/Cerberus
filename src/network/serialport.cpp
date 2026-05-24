#include "./serialport.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "src/cerberus.h"

#define DEFAULT_SERIAL_RECV_BUFFER_SIZE 512

using namespace crb;

static speed_t serialPort_getBaudrateFlag(uint32_t baudrate)
{
    switch (baudrate)
    {
        case 0:
            return B0;
        case 50:
            return B50;
        case 75:
            return B75;
        case 110:
            return B110;
        case 134:
            return B134;
        case 150:
            return B150;
        case 200:
            return B200;
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 1800:
            return B1800;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
#ifdef B460800
        case 460800:
            return B460800;
#endif
#ifdef B500000
        case 500000:
            return B500000;
#endif
#ifdef B576000
        case 576000:
            return B576000;
#endif
#ifdef B921600
        case 921600:
            return B921600;
#endif
#ifdef B1000000
        case 1000000:
            return B1000000;
#endif
#ifdef B1152000
        case 1152000:
            return B1152000;
#endif
#ifdef B1500000
        case 1500000:
            return B1500000;
#endif
#ifdef B2000000
        case 2000000:
            return B2000000;
#endif
#ifdef B2500000
        case 2500000:
            return B2500000;
#endif
#ifdef B3000000
        case 3000000:
            return B3000000;
#endif
#ifdef B3500000
        case 3500000:
            return B3500000;
#endif
#ifdef B4000000
        case 4000000:
            return B4000000;
#endif
        default:
            return 0;
    }
}

//=============================================================================
SerialPort::SerialPort()
    : m_fd(-1),
      m_device(),
      m_config(),
      m_recvBuffer(DEFAULT_SERIAL_RECV_BUFFER_SIZE)
{
}
//=============================================================================
SerialPort::SerialPort(const std::string& device)
    : m_fd(-1),
      m_device(device),
      m_config(),
      m_recvBuffer(DEFAULT_SERIAL_RECV_BUFFER_SIZE)
{
}
//=============================================================================
SerialPort::SerialPort(SerialPort&& other)
    : m_fd(other.m_fd),
      m_device(std::move(other.m_device)),
      m_config(other.m_config),
      m_recvBuffer(std::move(other.m_recvBuffer))
{
    other.m_fd = -1;
}
//=============================================================================
SerialPort::~SerialPort() { close(); }
//=============================================================================
SerialPort& SerialPort::operator=(SerialPort&& other)
{
    if (this == &other) return *this;

    close();

    m_fd         = other.m_fd;
    m_device     = std::move(other.m_device);
    m_config     = other.m_config;
    m_recvBuffer = std::move(other.m_recvBuffer);

    other.m_fd = -1;

    return *this;
}
//=============================================================================
OpRes SerialPort::applyConfig()
{
    if (!isOpen()) return OR_FailedInstance;

    speed_t baudFlag = serialPort_getBaudrateFlag(m_config.baudrate);
    if (baudFlag == 0)
    {
        return {OR_WrongArgument, CerberusUtils::strPrint("unsupported serial baudrate %u", m_config.baudrate)};
    }

    termios tty{};
    if (::tcgetattr(m_fd, &tty) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial tcgetattr error, %s", strerror(errno))};
    }

    if (m_config.canonical)
    {
        tty.c_iflag &= ~(IGNBRK | INLCR | IGNCR | PARMRK | ISTRIP | INPCK | IXOFF | IXANY);
        tty.c_iflag |= BRKINT | ICRNL;
        tty.c_oflag |= OPOST;
        tty.c_lflag &= ~(ECHO | ECHONL);
        tty.c_lflag |= ICANON | ISIG | IEXTEN;
    }
    else
    {
        ::cfmakeraw(&tty);
    }

    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSIZE;

    switch (m_config.dataBits)
    {
        case 5:
            tty.c_cflag |= CS5;
            break;
        case 6:
            tty.c_cflag |= CS6;
            break;
        case 7:
            tty.c_cflag |= CS7;
            break;
        case 8:
            tty.c_cflag |= CS8;
            break;
        default:
            return {OR_WrongArgument, CerberusUtils::strPrint("unsupported serial data bits %u", m_config.dataBits)};
    }

    tty.c_cflag &= ~(PARENB | PARODD);
    switch (m_config.parity)
    {
        case SerialParity_None:
            break;
        case SerialParity_Even:
            tty.c_cflag |= PARENB;
            break;
        case SerialParity_Odd:
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
            break;
    }

    switch (m_config.stopBits)
    {
        case SerialStopBits_One:
            tty.c_cflag &= ~CSTOPB;
            break;
        case SerialStopBits_Two:
            tty.c_cflag |= CSTOPB;
            break;
        default:
            return {OR_WrongArgument, "unsupported serial stop bits"};
    }

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag &= ~CRTSCTS;

    switch (m_config.flowControl)
    {
        case SerialFlowControl_None:
            break;
        case SerialFlowControl_Software:
            tty.c_iflag |= IXON | IXOFF;
            break;
        case SerialFlowControl_Hardware:
            tty.c_cflag |= CRTSCTS;
            break;
        default:
            return {OR_WrongArgument, "unsupported serial flow control"};
    }

    if (m_config.canonical)
    {
        tty.c_cc[VMIN]  = 1;
        tty.c_cc[VTIME] = 0;
    }
    else
    {
        uint64_t vtimeMs = m_config.vtime.toMilliseconds();
        uint64_t vtimeDs = vtimeMs / 100;

        if ((vtimeMs % 100) != 0) vtimeDs++;

        if (vtimeDs > 255)
        {
            return {OR_WrongArgument, "serial vtime exceeds termios range"};
        }

        tty.c_cc[VMIN]  = m_config.vmin;
        tty.c_cc[VTIME] = (cc_t)vtimeDs;
    }

    if (::cfsetispeed(&tty, baudFlag) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial cfsetispeed error, %s", strerror(errno))};
    }

    if (::cfsetospeed(&tty, baudFlag) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial cfsetospeed error, %s", strerror(errno))};
    }

    if (::tcsetattr(m_fd, TCSANOW, &tty) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial tcsetattr error, %s", strerror(errno))};
    }

    int flags = ::fcntl(m_fd, F_GETFL, 0);
    if (flags == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial fcntl getfl error, %s", strerror(errno))};
    }

    if (m_config.readBlocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if (::fcntl(m_fd, F_SETFL, flags) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial fcntl setfl error, %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes SerialPort::recvInternal(ByteBuffer& buffer)
{
    if (!isOpen()) return OR_FailedInstance;

    buffer.clear();

    ssize_t ret = ::read(m_fd, m_recvBuffer.data(), m_recvBuffer.size());

    if (ret == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return OR_WouldBlock;
        if (errno == EINTR) return OR_TemporaryUnavailable;
        return {OR_SystemFailure, CerberusUtils::strPrint("serial read error, %s", strerror(errno))};
    }

    if (ret == 0) return OR_TimedOut;

    buffer.assign(m_recvBuffer, ret);
    return OR_OK;
}
//=============================================================================
OpRes SerialPort::setModemBit(int bit, bool state)
{
    if (!isOpen()) return OR_FailedInstance;

    int ret = 0;

    if (state)
        ret = ::ioctl(m_fd, TIOCMBIS, &bit);
    else
        ret = ::ioctl(m_fd, TIOCMBIC, &bit);

    if (ret == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial ioctl error, %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
BoolOpRes SerialPort::getModemBit(int bit) const
{
    if (!isOpen()) return OR_FailedInstance;

    int status = 0;
    if (::ioctl(m_fd, TIOCMGET, &status) == -1)
    {
        return {OpRes(OR_SystemFailure, CerberusUtils::strPrint("serial ioctl error, %s", strerror(errno)))};
    }

    return (status & bit) != 0;
}
//=============================================================================
OpRes SerialPort::open(const std::string& device, const SerialPortConfig& config)
{
    if (isOpen())
    {
        OpRes res = close();
        if (res.fail()) return res;
    }

    m_device = device;
    m_config = config;

    m_fd = ::open(m_device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd == -1)
    {
        Result result = OR_Failure;

        switch (errno)
        {
            case ENOENT:
            case ENODEV:
            case ENOTDIR:
                result = OR_InvalidPath;
                break;

            case EACCES:
            case EPERM:
                result = OR_BadConditions;
                break;

            default:
                result = OR_SystemFailure;
                break;
        }

        return {result, CerberusUtils::strPrint("serial open error on %s: %s", m_device.c_str(), strerror(errno))};
    }

    OpRes res = applyConfig();
    if (res.fail())
    {
        ::close(m_fd);
        m_fd = -1;
        return res;
    }

    return OR_OK;
}
//=============================================================================
OpRes SerialPort::open(const std::string& device, uint32_t baudrate)
{
    SerialPortConfig config;
    config.baudrate = baudrate;
    return open(device, config);
}
//=============================================================================
OpRes SerialPort::close()
{
    if (!isOpen()) return OR_FailedInstance;

    OpRes res(OR_OK);

    if (::close(m_fd) == -1)
    {
        res = {OR_SystemFailure, CerberusUtils::strPrint("serial close error, %s", strerror(errno))};
    }

    m_fd = -1;

    return res;
}
//=============================================================================
OpRes SerialPort::reset()
{
    if (m_device.empty()) return OR_FailedInstance;

    if (isOpen())
    {
        ::close(m_fd);
        m_fd = -1;
    }

    return open(m_device, m_config);
}
//=============================================================================
bool SerialPort::isOpen() const { return m_fd != -1; }
//=============================================================================
const std::string& SerialPort::device() const { return m_device; }
//=============================================================================
OpRes SerialPort::configure(const SerialPortConfig& config)
{
    m_config = config;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::setBaudrate(uint32_t baudrate)
{
    m_config.baudrate = baudrate;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::setDataBits(uint8_t bits)
{
    m_config.dataBits = bits;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::setParity(SerialParity parity)
{
    m_config.parity = parity;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::setStopBits(SerialStopBits bits)
{
    m_config.stopBits = bits;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::setFlowControl(SerialFlowControl flow)
{
    m_config.flowControl = flow;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::setCanonical(bool enable)
{
    m_config.canonical = enable;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::setReadMode(bool blocking, uint8_t vmin, const TimeFrame& vtime)
{
    m_config.readBlocking = blocking;
    m_config.vmin         = vmin;
    m_config.vtime        = vtime;
    return applyConfig();
}
//=============================================================================
OpRes SerialPort::send(const ByteBuffer& buffer, bool donotblock)
{
    if (!isOpen()) return OR_FailedInstance;

    size_t len = buffer.size();
    if (len == 0) return OR_OK;

    uint8_t* data = (uint8_t*)buffer.data();
    size_t off = 0;

    while (off < len)
    {
        ssize_t ret = ::write(m_fd, data + off, len - off);

        if (ret == -1)
        {
            if (errno == EINTR) continue;

            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (donotblock) return OR_WouldBlock;

                OpRes waitRes = waitWrite();
                if (waitRes.fail()) return waitRes;
                continue;
            }

            return {OR_SystemFailure, CerberusUtils::strPrint("serial write error, %s", strerror(errno))};
        }

        if (ret == 0) return {OR_Failure, "serial write returned 0"};

        off += ret;
    }

    return OR_OK;
}
//=============================================================================
OpRes SerialPort::recv(ByteBuffer& buffer, const TimeFrame& timeout)
{
    if (!isOpen()) return OR_FailedInstance;

    if (timeout.isNull()) return recvInternal(buffer);

    while (true)
    {
        OpRes waitRes = waitRead(timeout);
        if (waitRes.fail()) return waitRes;

        OpRes recvRes = recvInternal(buffer);

        if (recvRes == OR_TemporaryUnavailable) continue;
        if (recvRes == OR_WouldBlock) continue;

        return recvRes;
    }
}
//=============================================================================
OpRes SerialPort::recv(ByteBuffer& buffer)
{
    if (!isOpen()) return OR_FailedInstance;

    while (true)
    {
        OpRes waitRes = waitRead();
        if (waitRes.fail()) return waitRes;

        OpRes recvRes = recvInternal(buffer);

        if (recvRes == OR_TemporaryUnavailable) continue;
        if (recvRes == OR_WouldBlock) continue;

        return recvRes;
    }
}
//=============================================================================
OpRes SerialPort::canRead()
{
    if (!isOpen()) return OR_FailedInstance;

    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLIN;

    int ret = ::poll(&set, 1, 0);

    if (ret == 0) return OR_TimedOut;

    if (ret == -1)
    {
        if (errno == EINTR) return OR_TemporaryUnavailable;
        return {OR_SystemFailure, CerberusUtils::strPrint("error in poll: %s", strerror(errno))};
    }

    if (set.revents & POLLIN) return OR_OK;
    if (set.revents & POLLERR) return OR_Failure;
    if (set.revents & POLLHUP) return OR_Hangup;
    if (set.revents & POLLNVAL) return OR_BadConditions;

    return OR_Failure;
}
//=============================================================================
OpRes SerialPort::waitRead(const TimeFrame& timeout)
{
    if (!isOpen()) return OR_FailedInstance;

    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLIN;

    int ret = ::poll(&set, 1, timeout.isNull() ? -1 : timeout.toMilliseconds());

    if (ret == 0) return OR_TimedOut;

    if (ret == -1)
    {
        if (errno == EINTR) return OR_TemporaryUnavailable;
        return {OR_SystemFailure, CerberusUtils::strPrint("error in poll: %s", strerror(errno))};
    }

    if (set.revents & POLLIN) return OR_OK;
    if (set.revents & POLLERR) return OR_Failure;
    if (set.revents & POLLHUP) return OR_Hangup;
    if (set.revents & POLLNVAL) return OR_BadConditions;

    return OR_Failure;
}
//=============================================================================
OpRes SerialPort::waitWrite(const TimeFrame& timeout)
{
    if (!isOpen()) return OR_FailedInstance;

    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLOUT;

    int ret = ::poll(&set, 1, timeout.isNull() ? -1 : timeout.toMilliseconds());

    if (ret == 0) return OR_TimedOut;

    if (ret == -1)
    {
        if (errno == EINTR) return OR_TemporaryUnavailable;
        return {OR_SystemFailure, CerberusUtils::strPrint("error in poll: %s", strerror(errno))};
    }

    if (set.revents & POLLOUT) return OR_OK;
    if (set.revents & POLLERR) return OR_Failure;
    if (set.revents & POLLHUP) return OR_Hangup;
    if (set.revents & POLLNVAL) return OR_BadConditions;

    return OR_Failure;
}
//=============================================================================
OpRes SerialPort::flushInput()
{
    if (!isOpen()) return OR_FailedInstance;

    if (::tcflush(m_fd, TCIFLUSH) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial tcflush input error, %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes SerialPort::flushOutput()
{
    if (!isOpen()) return OR_FailedInstance;

    if (::tcflush(m_fd, TCOFLUSH) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial tcflush output error, %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes SerialPort::flush()
{
    if (!isOpen()) return OR_FailedInstance;

    if (::tcflush(m_fd, TCIOFLUSH) == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("serial tcflush error, %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes SerialPort::setDTR(bool state) { return setModemBit(TIOCM_DTR, state); }
//=============================================================================
OpRes SerialPort::setRTS(bool state) { return setModemBit(TIOCM_RTS, state); }
//=============================================================================
BoolOpRes SerialPort::getCTS() const { return getModemBit(TIOCM_CTS); }
//=============================================================================
BoolOpRes SerialPort::getDSR() const { return getModemBit(TIOCM_DSR); }
//=============================================================================
BoolOpRes SerialPort::getDCD() const { return getModemBit(TIOCM_CAR); }
//=============================================================================
BoolOpRes SerialPort::getRI() const { return getModemBit(TIOCM_RI); }
