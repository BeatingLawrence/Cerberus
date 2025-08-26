#ifndef CERBERUS_TYPES_H
#define CERBERUS_TYPES_H

#ifndef WINDOWS_SYSTEM
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <atomic>
#include <cstdint>
#include <cstring>
#include <list>
#include <string>
#include <type_traits>
#include <vector>

#include "exception/exception.h"
#include "thread/mutexlocker.h"
#include "time/datetime.h"

#define StringOpRes ::cerberus::OpResData<std::string>
#define BoolOpRes ::cerberus::OpResData<bool>
#define IntOpRes ::cerberus::OpResData<int64_t>
#define FloatOpRes ::cerberus::OpResData<long double>
#define SizeOpRes ::cerberus::OpResData<::cerberus::LSIZE>

#define condret(cond)                       \
    {                                       \
        auto _result = cond;                \
        if (_result.fail()) return _result; \
    }

#define condret_str(cond, str)                     \
    {                                              \
        auto _result = cond;                       \
        if (_result.fail()) return {_result, str}; \
    }

namespace cerberus
{
    typedef uint32_t SIZE;
    typedef uint64_t LSIZE;
    typedef uint8_t BYTE;
    typedef int64_t OFFSET;
    typedef uint32_t HASH32;
    typedef HASH32 CHANDLE;
    typedef uint64_t DBMOD;

    struct VAR_256_BITS
    {
        BYTE x[32];

        VAR_256_BITS()
            : x() {};

        VAR_256_BITS(uint64_t val)
            : x()
        {
            memmove(x, &val, sizeof(val));
        };

        VAR_256_BITS(const char* str)
            : x()
        {
            if (!str) return;
            for (uint8_t i = 0; i < sizeof(x); i++)
            {
                if (!(*str)) return;
                x[i] = *str;
                str++;
            }
        };

        VAR_256_BITS(void* buf, size_t len)
            : x()
        {
            if (len > sizeof(x)) len = sizeof(x);
            memmove(x, buf, len);
        };

        VAR_256_BITS(const VAR_256_BITS& other) = default;

        VAR_256_BITS& operator=(const VAR_256_BITS&) = default;

        const unsigned char* p() const { return x; };
    };

    typedef VAR_256_BITS HASH256;
    typedef VAR_256_BITS KEY256;

    struct bitmask
    {
        bitmask(uint8_t bits = 0)
            : bits(bits) {};

        uint8_t bits;

        // bit 0 is LSB
        bool operator[](uint8_t i)
        {
            if (i > 7) return false;
            return bits & ((uint8_t)0x1 << i);
        };

        void set(uint8_t i, bool val = true)
        {
            if (i > 7) return;
            if (val)
                bits |= (uint8_t)0x1 << i;
            else
                bits &= ~((uint8_t)0x1 << i);
        };

        void reset() { bits = 0; };
    };

    enum DBDataType : uint8_t
    {
        DDT_Undefined = 0,
        DDT_BigInt    = 1,   // [fix] 8 byte signed integer
        DDT_Int       = 2,   // [fix] 4 byte signed integer
        DDT_SmallInt  = 3,   // [fix] 2 byte signed integer
        DDT_Real      = 4,   // [fix] 4 byte signed float
        DDT_Double    = 5,   // [fix] 8 byte signed float
        DDT_Boolean   = 6,   // [fix] bool (1 byte)
        DDT_Bit       = 7,   // [fix] fixed length bit array (needs modifier)
        DDT_VarBit    = 8,   // [var] variable length bit array (needs modifier)
        DDT_Char      = 9,   // [fix] fixed length char array (needs modifier)
        DDT_VarChar   = 10,  // [var] variable length char array (needs modifier)
        DDT_Money     = 11,  // [fix] fixed fractional precision (2 digits typically, 4 bytes)
    };

    enum DBBackend : uint8_t
    {
        DBB_PostgreSQL,
        DBB_Filesystem,
    };

    enum MemorySize : size_t
    {
        MEM_1K   = 1024,
        MEM_2K   = 2048,
        MEM_4K   = 4096,
        MEM_8K   = 8192,
        MEM_16K  = 16384,
        MEM_32K  = 32768,
        MEM_64K  = 65536,
        MEM_128K = 131072,
        MEM_256K = 262144,
        MEM_512K = 524288,
    };

    struct Path : private std::list<std::string>
    {
        bool absolute;

        Path()
            : absolute(false) {};

        Path(const std::string& path)
            : absolute(false)
        {
            fromStr(path);
        }

        Path(const char* path)
            : absolute(false)
        {
            fromStr(path);
        }

        using std::list<std::string>::back;
        using std::list<std::string>::empty;
        using std::list<std::string>::pop_back;
        using std::list<std::string>::pop_front;

        void clear()
        {
            absolute = false;
            std::list<std::string>::clear();
        };

        void append(const std::string& str) { push_back(str); }

        Path copy_append(const std::string& str) const
        {
            Path p(*this);
            p.append(str);
            return p;
        }

        std::string toStr() const
        {
            std::string ret;

            if (absolute) ret.append("/");

            for (auto&& el : *this)
            {
                ret.append(el);
                ret.append("/");
            }

            ret.pop_back();  // remove last '/'

            return ret;
        }

        void fromStr(const std::string& str)
        {
            clear();
            std::string tmp;

            absolute = str.front() == '/';

            for (auto& el : str)
            {
                if (el == '/')
                {
                    if (!tmp.empty()) push_back(tmp);  // TODO USE REGEX
                    tmp.clear();
                }
                else
                    tmp.push_back(el);
            }

            if (!tmp.empty()) push_back(tmp);
        }
    };

    template <class T>
    class Iterator
    {
        T* p;

       public:
        Iterator(T* x)
            : p(x)
        {
        }

        // Prefix increment
        Iterator& operator++()
        {
            p++;
            return *this;
        }

        // Postfix increment
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) { return a.p == b.p; };
        friend bool operator!=(const Iterator& a, const Iterator& b) { return a.p != b.p; };
        T& operator*() const { return *p; }
        T* operator->() { return p; };
    };

    template <class T>
    class ConstIterator
    {
        const T* p;

       public:
        ConstIterator(const T* x)
            : p(x)
        {
        }

        // Prefix increment
        ConstIterator& operator++()
        {
            p++;
            return *this;
        }

        // Postfix increment
        ConstIterator operator++(int)
        {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const ConstIterator& a, const ConstIterator& b) { return a.p == b.p; };
        friend bool operator!=(const ConstIterator& a, const ConstIterator& b) { return a.p != b.p; };
        const T& operator*() const { return *p; }
        const T* operator->() { return p; };
    };

    struct DoubleString
    {
        std::string left;
        std::string right;
    };

    struct LoaderFunc
    {
        void* func;
        MutexLocker mutexLocker;

        bool isValid() { return func != nullptr; };
    };

    struct TLS_SD_STATE
    {
        bool local;
        bool remote;

        TLS_SD_STATE(bool local = false, bool remote = false)
            : local(local),
              remote(remote)
        {
        }

        bool any() { return local || remote; }
    };

    struct CerbVersion
    {
        enum VersionType : uint8_t
        {
            Alpha,
            Beta,
            Release,
        };

        uint16_t major;
        uint16_t minor;
        uint16_t patch;
        VersionType type;
        std::string text;
    };

    enum FileType : uint8_t
    {
        FT_BLK,
        FT_CHR,
        FT_DIR,
        FT_FIFO,
        FT_LNK,
        FT_REG,
        FT_SOCK,
    };

    enum FilePermission : uint8_t
    {
        FP_UID    = 32,
        FP_GID    = 16,
        FP_STICKY = 8,
        FP_READ   = 4,
        FP_WRITE  = 2,
        FP_EXEC   = 1,
    };

    struct FileMode
    {
        uint8_t user;
        uint8_t group;
        uint8_t other;
    };

    struct FileMetadata
    {
        FileType type;     // the file type
        SIZE linkrefs;     // the number of refs to the hard link
        LSIZE size;        // the file size
        FileMode mode;     // the file mode
        LSIZE ownUID;      // the owner user ID
        LSIZE ownGID;      // the owner group ID
        DateTime accTime;  // last file access time
        DateTime modTime;  // last modification time
        DateTime chgTime;  // last status change time
        DateTime creTime;  // file creation time

#if defined(LINUX_SYSTEM)
        void fromStat(const struct statx& stat_struct);
#elif defined(APPLE_SYSTEM)
        void fromStat(const struct stat& stat_struct);
#endif
    };

    enum WordMatch : uint8_t
    {
        WM_CaseSensitive,
        WM_CaseInsensitive,
    };

    enum FileOpenMode : uint8_t
    {
        FOM_Read = 0,         // Open the file for reading only; the file must exist
        FOM_ReadWrite,        // Open the file for reading and writing; the file must exist
        FOM_ReadWriteTrunc,   // Open the file for reading and writing; if the file exists the
                              // content is discarded, otherwise, the file is created
        FOM_ReadWriteAppend,  // Open the file for reading and writing; if the file does not esist,
                              // it is created. All the write operations happen at the end of the
                              // file
    };

    enum ThreadPeriodicity : uint8_t
    {
        TP_Message,
        TP_Periodic,
        TP_PeriodicMessage,
        TP_OneShot,
        TP_Continuos,
        TP_Trigger,
    };

    enum SocketType : uint8_t
    {
        Socket_None,
        Socket_UDP,
        Socket_TCP,
        Socket_TCPP2P,
        Socket_ICMP,
        Socket_IPC,
    };

    enum Radix : uint8_t
    {
        Decimal,
        Hexadecimal,
        Binary,
    };

    enum LogLevel : int8_t
    {
        LL_Silent  = -1,
        LL_Info    = 0,
        LL_Warning = 1,
        LL_Error   = 2,
        LL_Debug   = 3,
    };

    enum SSLShutdownState : uint8_t
    {
        SSLSH_None,
        SSLSH_Sent,
        SSLSH_Received,
        SSLSH_SentReceived,
    };

    struct LogRole
    {
        uint8_t textFormatting[3];  // up to 3 formatting specifiers, 0 will be ignored, see define.h
        uint8_t foregroundColor;    // color specifier
        uint8_t backgroundColor;    // color specifier
    };

    struct FileLoggingConf
    {
        bool enable;              // enable the log on file
        std::string fileName;     // log file name
        LSIZE fileMaxSize;        // set to zero to disable log rotation (not recommended)
        std::string logDir;       // the archive of the log files
        std::string fileNameFmt;  // format string for the archived files name (see below)
        SIZE logDirMaxSize;       // the maximum size allowed for the log archive, before the
                                  // log deletion is performed. To disable this, set a value of zero
    };

    /* archive file format tokens:

       %h: hours
       %m: minutes
       %s: seconds

       %D: days
       %M: months
       %Y: years

     */

    struct LogConf
    {
        LogLevel appLogLevel;         // application log level. Minor levels will be silenced
        LogLevel cerbLogLevel;        // framework log level. Minor levels will be silenced
        bool colorFormatting;         // enable the color formatting of the output terminal
        LogRole infoRole;             // log role for the info level
        LogRole warningRole;          // log role for the warning level
        LogRole errorRole;            // log role for the error level
        LogRole debugRole;            // log role for the debug level
        FileLoggingConf fileLogConf;  // file logging configuration parameters
    };

    struct CoreConf
    {
        SIZE threadPool;  // set the size of the Cerberus thread pool. a value of zero disables it
        uint32_t backupThreadMaxTime;  // set the maximum time to keep backup threads alive (in ms)

        CoreConf()
            : threadPool(0),
              backupThreadMaxTime(0) {};
    };

    struct CerberusInitConf
    {
        LogConf logSetup;
        CoreConf coreSetup;
        bool useCiphers;  // enable cerberus to init and use the OpenSSL library
    };

    enum HTTPVersion : uint8_t
    {
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2,
    };

    enum HTTPMethod : uint8_t
    {
        HTTP_GET,
        HTTP_POST,
        HTTP_HEAD,
        HTTP_PUT,
        HTTP_DELETE,
        HTTP_PATCH,
        HTTP_TRACE,
        HTTP_OPTIONS,
        HTTP_CONNECT,
    };

    struct Clonable
    {
        virtual ~Clonable() {}
        virtual Clonable* clone() const = 0;  // return a copy-constructed instance of this
        virtual SIZE memfp() const      = 0;  // return the memory footprint of this
    };

    // -------------------------------------------------------------------------------
    // -------------------------------MANAGED PTR-------------------------------------
    // -------------------------------------------------------------------------------

    template <typename T>
    class managed_ptr
    {
        T* m_ptr;
        size_t* m_refcount;

        void _destroy()
        {
            if (!consistent()) return;

            if ((*m_refcount) == 1)
            {
                if (m_ptr) delete m_ptr;
                delete m_refcount;
            }
            else
                (*m_refcount)--;

            m_ptr      = nullptr;
            m_refcount = nullptr;
        }

        void _check() const
        {
            if (!consistent()) throw cIllegalStateExc("managed_ptr not valid");
        }

       public:
        bool consistent() const noexcept { return m_ptr && m_refcount; }

        explicit managed_ptr(T* ptr = nullptr)
            : m_ptr(ptr),
              m_refcount(nullptr)
        {
            if (m_ptr)
            {
                if (dynamic_cast<T*>(m_ptr) == nullptr) throw cInvalidCastExc("invalid pointer type");

                if (dynamic_cast<Clonable*>(m_ptr) == nullptr)
                    throw cInvalidCastExc("pointer is not a Clonable");

                m_refcount = new size_t(1);
                if (!consistent()) throw cSystemExc("failed allocation of refcounter");
            }
        }

        // copy constructor make a shallow copy
        managed_ptr(const managed_ptr<T>& other)
            : m_ptr(other.m_ptr),
              m_refcount(other.m_refcount)
        {
            if (consistent()) (*m_refcount)++;
        }

        managed_ptr(managed_ptr<T>&& other)
            : m_ptr(other.m_ptr),
              m_refcount(other.m_refcount)
        {
            other.m_ptr      = nullptr;
            other.m_refcount = nullptr;
        }

        // Move assignment operator
        managed_ptr& operator=(managed_ptr&& other) noexcept
        {
            if (this == &other) return *this;

            _destroy();

            m_ptr      = other.m_ptr;
            m_refcount = other.m_refcount;

            other.m_ptr      = nullptr;
            other.m_refcount = nullptr;
            return *this;
        }

        // shared copy
        managed_ptr<T>& operator=(const managed_ptr<T>& other) noexcept
        {
            if (this == &other) return *this;

            _check();
            _destroy();

            m_ptr      = other.m_ptr;
            m_refcount = other.m_refcount;

            if (consistent()) (*m_refcount)++;
            return *this;
        }

        ~managed_ptr() { _destroy(); }

        // duplicate the data (deep copy)
        managed_ptr<T> duplicate() const
        {
            if (consistent())
            {
                T* p = dynamic_cast<T*>(static_cast<Clonable*>(m_ptr)->clone());
                if (!p) throw cInvalidCastExc("cloned type is not valid");
                return managed_ptr<T>(p);
            }
            else
                return managed_ptr<T>();  // return unconsistent pointer
        }

        T* get() const { return m_ptr; }

        T& operator*() const
        {
            _check();
            return *(get());
        }

        T* operator->() const
        {
            _check();
            return get();
        }

        // stops managing the pointer, returning the wrapped underlying pointer
        T* disown()
        {
            _check();

            // assert that the instance count equals 1
            if (*m_refcount != 1) cIllegalStateExc("a disowning managed_ptr must be unique");

            delete m_refcount;
            m_refcount = nullptr;

            T* ret = get();
            m_ptr  = nullptr;
            return ret;
        }

        SIZE memFootprint() const
        {
            SIZE s = static_cast<Clonable*>(m_ptr)->memfp();
            return s + sizeof(managed_ptr);
        }
    };

    // -------------------------------------------------------------------------------
    // -----------------------------UNCLONABLE PTR------------------------------------
    // -------------------------------------------------------------------------------

    template <typename T>
    class unclonable_ptr
    {
        T* m_ptr;
        size_t* m_refcount;

        void _destroy()
        {
            if (!consistent()) return;

            if ((*m_refcount) == 1)
            {
                if (m_ptr) delete m_ptr;
                delete m_refcount;
            }
            else
                (*m_refcount)--;

            m_ptr      = nullptr;
            m_refcount = nullptr;
        }

        void _check() const
        {
            if (!consistent()) throw cIllegalStateExc("unclonable_ptr not valid");
        }

       public:
        bool consistent() const noexcept { return m_ptr && m_refcount; }

        explicit unclonable_ptr(T* ptr = nullptr)
            : m_ptr(ptr),
              m_refcount(nullptr)
        {
            if (m_ptr)
            {
                if (dynamic_cast<T*>(m_ptr) == nullptr) throw cInvalidCastExc("invalid pointer type");

                m_refcount = new size_t(1);
                if (!consistent()) throw cSystemExc("failed allocation of refcounter");
            }
        }

        // copy constructor make a shallow copy
        unclonable_ptr(const unclonable_ptr<T>& other)
            : m_ptr(other.m_ptr),
              m_refcount(other.m_refcount)
        {
            if (consistent()) (*m_refcount)++;
        }

        unclonable_ptr(unclonable_ptr<T>&& other)
            : m_ptr(other.m_ptr),
              m_refcount(other.m_refcount)
        {
            other.m_ptr      = nullptr;
            other.m_refcount = nullptr;
        }

        // Move assignment operator
        unclonable_ptr& operator=(unclonable_ptr&& other) noexcept
        {
            if (this == &other) return *this;

            _destroy();

            m_ptr      = other.m_ptr;
            m_refcount = other.m_refcount;

            other.m_ptr      = nullptr;
            other.m_refcount = nullptr;
            return *this;
        }

        // shared copy
        unclonable_ptr<T>& operator=(const unclonable_ptr<T>& other) noexcept
        {
            if (this == &other) return *this;

            _check();
            _destroy();

            m_ptr      = other.m_ptr;
            m_refcount = other.m_refcount;

            if (consistent()) (*m_refcount)++;
            return *this;
        }

        ~unclonable_ptr() { _destroy(); }

        T* get() const { return m_ptr; }

        T& operator*() const
        {
            _check();
            return *(get());
        }

        T* operator->() const
        {
            _check();
            return get();
        }

        // stops managing the pointer, returning the wrapped underlying pointer
        T* disown()
        {
            _check();

            // assert that the instance count equals 1
            if (*m_refcount != 1) cIllegalStateExc("a disowning unclonable_ptr must be unique");

            delete m_refcount;
            m_refcount = nullptr;

            T* ret = get();
            m_ptr  = nullptr;
            return ret;
        }
    };

    class Message;
    class BaseSlot;

    typedef managed_ptr<::cerberus::Message> cerberus_message;
    typedef managed_ptr<const ::cerberus::Message> cerberus_const_message;
    typedef managed_ptr<::cerberus::BaseSlot> slot_ptr;

    class Socket;

    typedef unclonable_ptr<::cerberus::Socket> cerberus_socket;

    enum IniDataType : uint8_t
    {
        IDT_Invalid = 0,  // specified when a value has an unknown type
        IDT_Integer = 2,  // false if key value contains a letter or a symbol
        IDT_Double  = 3,  // false if key value does not contain a '.' or if it contains a letter
        IDT_Bool    = 4,  // true only if key value equals "true" or "false" (case insensitive)
    };

    enum JsonDataType : uint8_t
    {
        // invalid Json:
        JDT_Undefined = 0,

        // valid Json:
        JDT_Null,
        JDT_Number,
        JDT_String,
        JDT_Boolean,
        JDT_Array,
        JDT_Object,
    };

    // The Result enum contains all the possible results of operation requested to the framework.
    enum Result : uint8_t
    {
        OR_Undefined,                 // [general] this result should never be given (used for unimplemented
                                      // methods)
        OR_OK,                        // [general] no errors
        OR_Failure,                   // [general] generic failure
        OR_FailedInstance,            // [general] attempt to use a failed instance
        OR_WouldBlock,                // [general] attempt to run a blocking operation on a non-blocking call
        OR_TimedOut,                  // [general] operation timeout
        OR_Unavailable,               // [general] the requested operation is not available for the object
        OR_WrongArgument,             // [general] at least one wrong argument has been given to the method
        OR_WrongData,                 // [general] wrong data has been provided/retrieved
        OR_InvalidPath,               // [general] the file does not exist or the given path is not valid
        OR_SystemFailure,             // [general] a system error occurred
        OR_BadConditions,             // [general] bad conditions encountered when processing the operation
        OR_NotFound,                  // [general] the item was not found
        OR_TemporaryUnavailable,      // [general] the requested operation is not available at the moment
        OR_NotEnoughData,             // [general] the requested operation requires more data
        OR_InvalidFile,               // [general] the provided file instance is not valid
        OR_Duplicate,                 // [general] the item is a duplicate
        OR_WrongType,                 // [general] the item type is wrong
        OR_NotEmpty,                  // [general] the item is not empty
        OR_Empty,                     // [general] the item is empty
        OR_Mismatch,                  // [general] the item does not match
        OR_AlreadyPresent,            // [general] the item is already present
                                      //
        OR_EOF,                       // [file] EOF reached
                                      //
        OR_ResolveServerTempFailure,  // [DNS lookup] resolve method error
        OR_ResolveServerFailure,      // [DNS lookup] resolve method error
        OR_ResolveNoData,             // [DNS lookup] resolve method error
        OR_ResolveNotFound,           // [DNS lookup] resolve method error
        OR_ResolveSystemFailure,      // [DNS lookup] resolve method error
        OR_ResolveFailure,            // [DNS lookup] resolve method error
                                      //
        OR_Hangup,                    // [socket] hangup condition (stream closed by the peer) or recv zero
        OR_TLSKeysCheckFail,          // [socket] the check procedure of the TLS keys has failed
                                      //
        OR_QueryFailure,              // [database] query error
        OR_DBFailure,                 // [database] DB error
                                      //
        OR_ThreadNotJoinable,         // [thread] the thread is not joinable
    };

    enum MutexType : uint8_t
    {
        Simple,     // ERRORCHECK mutex
        Recursive,  // RECURSIVE mutex
    };

    class Socket;

    struct SocketCloser
    {
        Socket* socket;

        SocketCloser(Socket* s = nullptr)
            : socket(s)
        {
        }

        SocketCloser(const SocketCloser& other)            = delete;
        SocketCloser& operator=(const SocketCloser& other) = delete;

        void assignSocket(Socket* s) { socket = s; }

        ~SocketCloser();
    };

    // The basic Operation Result object contains a Result member and some data.
    struct OpRes
    {
        Result res;

        std::string reason;

        std::vector<Result> optional;

        OpRes();

        OpRes(Result r, const std::string& reason = "", const std::string& reason2 = "");

        OpRes(const OpRes& opres, const std::string& reason = "");

        bool operator==(const OpRes& other) = delete;
        bool operator!=(const OpRes& other) = delete;

        bool operator==(Result r);
        bool operator!=(Result r);

        // Throw a generic exception with the given text only if the result is failed
        OpRes& expect(const std::string& str);

        // Throw a generic exception with the given text only if the result fails and
        // only if it fails with the given reason
        OpRes& expect(Result reason, const std::string& str);

        // Throw a generic exception with the result status text only if the result fails
        OpRes& expect();

        // Return true if the Result is OR_OK, false otherwise.
        // If print is specified, the error string will be printed with the internal error info
        bool ok(const std::string& str = "");

        // Return false if the Result is OR_OK, true otherwise.
        // If print is specified, the error string will be printed with the internal error info
        bool fail(const std::string& str = "");

        // Translate the Result variable into a string
        std::string errorString() const;

        // Translate the entire Result into a string
        std::string toStr() const;

        OpRes& addOptional(Result opt);

        bool hasOptional(Result opt);

        OpRes& addInfo(const std::string& str);

        SIZE memfp() const;
    };

    // The OpResData template class is useful to exchange some custom data alongside with the result
    template <class T>
    struct OpResData : OpRes
    {
        T value;

        OpResData(Result r)
            : OpRes(r),
              value() {};

        OpResData(const T& value)
            : OpRes(OR_OK),
              value(value) {};

        OpResData(const OpRes& res, const std::string& reason = "", const std::string& reason2 = "")
            : OpRes(res),
              value()
        {
            addInfo(reason);
            addInfo(reason2);
        };

        OpResData<T>& expect(const std::string& str)
        {
            OpRes::expect(str);
            return *this;
        };

        OpResData<T>& expect(Result reason, const std::string& str)
        {
            OpRes::expect(reason, str);
            return *this;
        };

        OpResData<T>& expect()
        {
            OpRes::expect();
            return *this;
        };
    };

    class Thread;

    typedef void (*timerCallback)(void* ctx);
    typedef int (*threadTickCallback)(cerberus_message msg, Thread* thr);
    typedef void (*threadCallback)(Thread* thr);
    typedef OpRes (*playerCallback)(void* ctx, void* data);
    typedef void (*taskEndCallback)(void* ctx, void* data, OpRes result);

    struct TimerData
    {
        DateTime delay;
        TimeFrame time;
        std::atomic_bool* bit;
        timerCallback callback;
        void* ctx;

        bool isPeriodic() { return !time.isNull(); }
        bool isDelayed() { return delay.isValid(); }
        bool isValid() { return (isPeriodic() || isDelayed()) && bit; }
    };

    enum TimerType : uint8_t
    {
        TT_OneShot,   // timer counts until expiry and stops (default)
        TT_Periodic,  // timer counts until expiry and then it restarts counting (it never stops)
        TT_Delayed,   // timer counts until delay time and then it counts the time (it never stops)
        TT_Alarm,     // timer counts until delay time and stops
    };

    struct Task
    {
        playerCallback cb;
        void* ctx;
        void* data;

        bool isValid() { return cb; };
    };

    struct DictLine
    {
        std::string key, val;
        SIZE sz() const { return key.capacity() + val.capacity() + (sizeof(std::string) * 2); }
    };

    class Dictionary : public std::vector<DictLine>
    {
       public:
        // Get the value of a field as text.
        // This method will return OR_NotFound if the requested field name was not found.
        StringOpRes getFieldValue(const std::string& key, WordMatch match = WM_CaseSensitive) const;

        // Check if a field value matches with value
        // This method will return OR_OK if the key value of the dictionary matches against
        // the provided value argument (the match policy is specified in the valmatch argument),
        // OR_NotFound if the requested field name was not found,
        // OR_Mismatch if the field name was found but it does not match with the specified value.
        OpRes getFieldMatch(const std::string& key, const std::string& value,
                            WordMatch keymatch = WM_CaseSensitive,
                            WordMatch valmatch = WM_CaseSensitive) const;

        // Get the name of the field at the index position.
        // An exception is thrown if index is out of bounds
        std::string getNameAt(SIZE index) const;

        // Get the value of the field at the index position.
        // An exception is thrown if index is out of bounds
        std::string getValueAt(SIZE index) const;

        // Add a key at the end of the dictionary
        Dictionary& addKey(const std::string& key, const std::string& value);

        // Get a line by reference
        // An exception is thrown if index is out of bounds
        DictLine& get(SIZE index);

        // Get a line by reference
        // An exception is thrown if key does not exist
        DictLine& get(const std::string& key, WordMatch match = WM_CaseSensitive);

        // Tell if a key is present in the dictionary
        bool exists(const std::string& key, WordMatch match = WM_CaseSensitive);

        // Print all the lines of the dictionary on a string
        std::string toString() const;

        // Return the allocated memory
        SIZE memfp() const;
    };

    struct Host
    {
        static const uint32_t ADDR_ANY;
        static const uint32_t ADDR_LOOPBACK;
        static const uint32_t ADDR_BROADCAST;

        union
        {
            uint8_t octect[4];  // e.g. address 192.168.4.5 has octect[0]=192 and octect[3]=5
            uint32_t octet_networkOrder;
        };

        uint16_t port;
        std::string hostname;

        bool resolved;

        // Construct an invalid Host (0.0.0.0:0)
        Host();

        // Construct an Host with str as hostname if it contains at least one letter,
        // otherwise str will be used to extract ip:port with a stringToHost() call.
        Host(const std::string& str);

        // Same as above
        Host(const char* str);

        // Take an ip address in the form of x.x.x.x or x.x.x.x:yyyyy
        // and return an Host object with numeric IP and port members filled
        // It is possible to use "any", "local" or "broadcast" (case insensitive) in place of the
        // IP address to specify 'any interface', 'localhost' and '255.255.255.255' respectively.
        // An invalid Host is returned if the conversion fails
        static Host stringToHost(const std::string& str);

        // Extract the port number from the given string.
        // The port number must be the last element in the string and must
        // be preceded by a column :
        // If no port is found, 0 is returned
        static uint16_t getPort(const std::string& str);

        // Extract a numeric IP address and port and saves them in this Host instance.
        // The instance becomes invalid if the conversion fails
        bool fromString(const std::string& str);

        // Print the numeric IP address and port (not the hostname) to string.
        std::string toString() const;

        // Tells if the Host is valid
        bool isValid() const;

        // Tell if the Host is valid for remote usage, e.g. connect() or sendTo().
        // For this method to return true, the Host must have a valid port
        // and either an hostname OR a numerical IP address
        bool isValidRemote() const;

        // Tell if the Host has a valid numerical IP
        bool isNumeric() const;

        // Tell if the Host has an hostname
        bool isTextual() const;

        // Tell if the Host has a valid port (port != 0)
        bool hasPort() const;

        // Resolve the given Host using the hostname member.
        // The resulting numeric IP address is written in the ip parameter
        OpRes resolve();
    };

    enum SocketTransfer : uint8_t
    {
        Transfer_Bytes,  // the socket will receive up to buffersize bytes
        Transfer_Time,   // the socket will keep calling recv() until timeout is reached
    };

    struct SocketSettings
    {
        SocketType type;              // type of socket (Socket_TCP or Socket_UDP)
        Host bind;                    // interface to bind the socket to
        Host remote;                  // remote host to keep the connection with
        SocketTransfer transferMode;  // socket data transfer mode

        TimeFrame tout, cyctout;  // timeout values used for Transfer_time
        SIZE maxpayload;          // recv buffer size of the socket

        bool server;     // true if the socket is a server socket (passive)
        size_t maxconn;  // maximum number of pending connection (for passive sockets only)

        SocketSettings()
            : type(Socket_TCP),
              transferMode(Transfer_Bytes),
              maxpayload(0),
              server(false),
              maxconn(0) {};
    };

    class ByteBuffer;
    class JsonData;

}  // namespace cerberus
#endif  // CERBERUS_TYPES_H
