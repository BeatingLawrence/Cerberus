#ifndef CERBERUS_NETWORK_SERIALPORT_H
#define CERBERUS_NETWORK_SERIALPORT_H

#include <string>

#include "../data/bytebuffer.h"
#include "../time/timeframe.h"
#include "../types.h"

namespace crb
{
    enum SerialParity : uint8_t
    {
        SerialParity_None,
        SerialParity_Even,
        SerialParity_Odd,
    };

    enum SerialStopBits : uint8_t
    {
        SerialStopBits_One,
        SerialStopBits_Two,
    };

    enum SerialFlowControl : uint8_t
    {
        SerialFlowControl_None,
        SerialFlowControl_Software,
        SerialFlowControl_Hardware,
    };

    struct SerialPortConfig
    {
        uint32_t baudrate;
        uint8_t dataBits;
        SerialParity parity;
        SerialStopBits stopBits;
        SerialFlowControl flowControl;
        bool canonical;
        bool readBlocking;
        uint8_t vmin;
        TimeFrame vtime;

        SerialPortConfig()
            : baudrate(9600),
              dataBits(8),
              parity(SerialParity_None),
              stopBits(SerialStopBits_One),
              flowControl(SerialFlowControl_None),
              canonical(false),
              readBlocking(false),
              vmin(0),
              vtime()
        {
        }
    };

    class SerialPort
    {
       private:
        int m_fd;
        std::string m_device;
        SerialPortConfig m_config;
        ByteBuffer m_recvBuffer;

        // Apply the cached configuration to the currently opened device.
        OpRes applyConfig();

        // Read immediately from the port without any extra waiting logic.
        OpRes recvInternal(ByteBuffer& buffer);

        // Set or clear a modem-control output bit such as DTR or RTS.
        OpRes setModemBit(int bit, bool state);

        // Read the current state of a modem-control input bit.
        BoolOpRes getModemBit(int bit) const;

       public:
        // Construct a closed serial port with default configuration.
        SerialPort();

        // Construct a closed serial port and cache the target device path.
        explicit SerialPort(const std::string& device);
        SerialPort(const SerialPort& other) = delete;

        // Move-construct the port state from another instance.
        SerialPort(SerialPort&& other);

        // Close the serial port if still open.
        ~SerialPort();

        SerialPort& operator=(const SerialPort& other) = delete;

        // Move-assign the port state from another instance, closing the current one first.
        SerialPort& operator=(SerialPort&& other);

        // Open a serial device and apply the provided communication settings.
        OpRes open(const std::string& device, const SerialPortConfig& config = SerialPortConfig());

        // Open a serial device using the given baudrate and default settings for the rest.
        OpRes open(const std::string& device, uint32_t baudrate);

        // Close the currently opened serial device.
        OpRes close();

        // Close and reopen the same device, reapplying the cached configuration.
        OpRes reset();

        // Tell whether the serial device is currently opened.
        bool isOpen() const;

        // Return the cached device path associated with this serial port.
        const std::string& device() const;

        // Replace the entire cached configuration and apply it to the open device.
        OpRes configure(const SerialPortConfig& config);

        // Change only the baudrate and apply it immediately.
        OpRes setBaudrate(uint32_t baudrate);

        // Change only the number of data bits and apply it immediately.
        OpRes setDataBits(uint8_t bits);

        // Change only the parity mode and apply it immediately.
        OpRes setParity(SerialParity parity);

        // Change only the number of stop bits and apply it immediately.
        OpRes setStopBits(SerialStopBits bits);

        // Change only the flow-control mode and apply it immediately.
        OpRes setFlowControl(SerialFlowControl flow);

        // Enable or disable canonical line mode; default is disabled.
        OpRes setCanonical(bool enable);

        // Configure blocking/non-blocking reads and the termios VMIN/VTIME thresholds.
        OpRes setReadMode(bool blocking, uint8_t vmin = 0, const TimeFrame& vtime = TimeFrame());

        // Send the whole buffer on the serial line, optionally without blocking.
        OpRes send(const ByteBuffer& buffer, bool donotblock = false);

        // Wait up to timeout for incoming data and then read a chunk into buffer.
        OpRes recv(ByteBuffer& buffer, const TimeFrame& timeout);

        // Block until incoming data are available and then read a chunk into buffer.
        OpRes recv(ByteBuffer& buffer);

        // Poll the port once and report whether data are ready to be read.
        OpRes canRead();

        // Block until the port becomes readable or the timeout expires.
        OpRes waitRead(const TimeFrame& timeout = TimeFrame());

        // Block until the port becomes writable or the timeout expires.
        OpRes waitWrite(const TimeFrame& timeout = TimeFrame());

        // Discard all pending bytes currently queued in the input direction.
        OpRes flushInput();

        // Discard all pending bytes currently queued in the output direction.
        OpRes flushOutput();

        // Discard both pending input and pending output bytes.
        OpRes flush();

        // Drive the DTR modem-control output line to the requested state.
        OpRes setDTR(bool state);

        // Drive the RTS modem-control output line to the requested state.
        OpRes setRTS(bool state);

        // Return the current state of the CTS modem-control input line.
        BoolOpRes getCTS() const;

        // Return the current state of the DSR modem-control input line.
        BoolOpRes getDSR() const;

        // Return the current state of the DCD modem-control input line.
        BoolOpRes getDCD() const;

        // Return the current state of the RI modem-control input line.
        BoolOpRes getRI() const;
    };
}  // namespace crb

#endif  // CERBERUS_NETWORK_SERIALPORT_H
