#ifndef CERBERUS_TYPES_H
#define CERBERUS_TYPES_H

#if !defined(WINDOWS_SYSTEM) && !defined(_WIN32) && !defined(WIN32)
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "define.h"  // IWYU pragma: export
#include "exception/exception.h"
#include "thread/mutexlocker.h"
#include "time/datetime.h"

#define StringOpRes ::crb::OpResData<std::string>
#define BoolOpRes ::crb::OpResData<bool>
#define IntOpRes ::crb::OpResData<int64_t>
#define FloatOpRes ::crb::OpResData<long double>
#define SizeOpRes ::crb::OpResData<::crb::LSIZE>

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

namespace crb
{
    typedef uint32_t SIZE;
    typedef uint64_t LSIZE;
    typedef uint8_t BYTE;
    typedef int64_t OFFSET;
    typedef uint32_t DBMOD;

    constexpr DBMOD DBMOD_KEY_FLAG = 0x80000000u;  // primary key bit (highest bit)

    typedef std::vector<std::string> MultiString;

    class HASH32
    {
       public:
        constexpr HASH32(uint32_t value = 0) noexcept
            : m_value(value)
        {
        }

        constexpr HASH32(int value) noexcept
            : m_value(static_cast<uint32_t>(value))
        {
        }
        constexpr HASH32(uint64_t value) noexcept
            : m_value(static_cast<uint32_t>(value))
        {
        }
        constexpr HASH32(int64_t value) noexcept
            : m_value(static_cast<uint32_t>(value))
        {
        }

        CERBERUS_EXPORT HASH32(const char* str);
        CERBERUS_EXPORT HASH32(const std::string& str);

        HASH32& operator=(uint32_t value) noexcept
        {
            m_value = value;
            return *this;
        }

        HASH32& operator=(int value) noexcept
        {
            m_value = static_cast<uint32_t>(value);
            return *this;
        }
        HASH32& operator=(uint64_t value) noexcept
        {
            m_value = static_cast<uint32_t>(value);
            return *this;
        }
        HASH32& operator=(int64_t value) noexcept
        {
            m_value = static_cast<uint32_t>(value);
            return *this;
        }

        CERBERUS_EXPORT HASH32& operator=(const char* str);
        CERBERUS_EXPORT HASH32& operator=(const std::string& str);

        constexpr operator uint32_t() const noexcept { return m_value; }
        constexpr uint32_t value() const noexcept { return m_value; }

       private:
        uint32_t m_value;
    };

    using DBFLAGS = uint32_t;
    constexpr DBFLAGS DBF_None       = 0x00000000;
    constexpr DBFLAGS DBF_PrimaryKey = 0x00000001;

    constexpr inline bool operator==(HASH32 a, HASH32 b) noexcept { return a.value() == b.value(); }
    constexpr inline bool operator!=(HASH32 a, HASH32 b) noexcept { return a.value() != b.value(); }
    constexpr inline bool operator==(HASH32 a, uint32_t b) noexcept { return a.value() == b; }
    constexpr inline bool operator!=(HASH32 a, uint32_t b) noexcept { return a.value() != b; }
    constexpr inline bool operator==(uint32_t a, HASH32 b) noexcept { return a == b.value(); }
    constexpr inline bool operator!=(uint32_t a, HASH32 b) noexcept { return a != b.value(); }

    typedef HASH32 CHANDLE;

    struct CoreSet
    {
        std::vector<int> cores;

        void addCore(int core)
        {
            if (std::find(cores.begin(), cores.end(), core) != cores.end()) return;
            cores.push_back(core);
        }

        void removeCore(int core)
        {
            auto it = std::find(cores.begin(), cores.end(), core);
            if (it != cores.end()) cores.erase(it);
        }
        bool empty() const { return cores.empty(); }

        static CoreSet subtract(const CoreSet& a, const CoreSet& b)
        {
            CoreSet out;
            out.cores.reserve(a.cores.size());
            for (int core : a.cores)
            {
                if (std::find(b.cores.begin(), b.cores.end(), core) == b.cores.end())
                    out.cores.push_back(core);
            }
            return out;
        }

        // Create an empty CoreSet
        CoreSet() = default;

        // Create a CoreSet with just one core
        CoreSet(int core)
            : cores()
        {
            cores.push_back(core);
        };
    };

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

    enum UpdatePolicy : uint8_t
    {
        UP_UpdateOnly,
        UP_InsertOnly,
        UP_UpdateInsert,
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

            if (!ret.empty()) ret.pop_back();  // remove last '/'

            return ret;
        }

        void fromStr(const std::string& str)
        {
            clear();
            std::string tmp;

            if (str.empty())
            {
                absolute = false;
                return;
            }

#if defined(WINDOWS_SYSTEM) || defined(_WIN32) || defined(WIN32)
            absolute = str.front() == '/' || str.front() == '\\';
#else
            absolute = str.front() == '/';
#endif

            for (auto& el : str)
            {
#if defined(WINDOWS_SYSTEM) || defined(_WIN32) || defined(WIN32)
                const bool separator = el == '/' || el == '\\';
#else
                const bool separator = el == '/';
#endif
                if (separator)
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

    struct TLS_ShutdownState
    {
        bool local;
        bool remote;

        TLS_ShutdownState(bool local = false, bool remote = false)
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

    enum ThreadStackSize : LSIZE
    {
        TSS_4M   = 4ULL * 1024ULL * 1024ULL,
        TSS_8M   = 8ULL * 1024ULL * 1024ULL,
        TSS_16M  = 16ULL * 1024ULL * 1024ULL,
        TSS_32M  = 32ULL * 1024ULL * 1024ULL,
        TSS_64M  = 64ULL * 1024ULL * 1024ULL,
        TSS_128M = 128ULL * 1024ULL * 1024ULL,
        TSS_256M = 256ULL * 1024ULL * 1024ULL,
        TSS_512M = 512ULL * 1024ULL * 1024ULL,
    };

    enum ThreadPeriodicity : uint8_t
    {
        TP_Message,
        TP_Periodic,
        TP_Periodic_realtime,
        TP_PeriodicMessage,
        TP_OneShot,
        TP_Continuos,
        TP_Continuos_realtime,
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
        LogLevel fwLogLevel;          // framework log level. Minor levels will be silenced
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
        CoreSet coreSet;               // default core set for Cerberus threads (empty = no affinity)

        CoreConf()
            : threadPool(0),
              backupThreadMaxTime(0),
              coreSet() {};
    };

    struct CerberusInitConf
    {
        LogConf logSetup;
        CoreConf coreSetup;
        bool useCiphers;                   // enable cerberus to init and use the OpenSSL library
        std::string appConfigurationFile;  // application configuration file path
        bool initFromFile;                 // override init params from configuration file
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
        virtual ~Clonable()             = default;
        virtual Clonable* clone() const = 0;  // return a copy-constructed instance of this
        virtual LSIZE memfp() const     = 0;  // return the memory footprint of this
    };

    // -------------------------------------------------------------------------------
    // -------------------------------MANAGED PTR-------------------------------------
    // -------------------------------------------------------------------------------
    // ---------------------- A WRAPPER OF STD::UNIQUE_PTR ---------------------------

    template <typename T>
    class managed_ptr
    {
        static inline void _enforce_clonable()
        {
            static_assert(std::is_base_of_v<Clonable, std::remove_cv_t<T>>,
                          "managed_ptr<T>: T must derive from Clonable");
        }

        std::unique_ptr<T> _ptr;

        static T* clone_to_T(const T* p)
        {
            _enforce_clonable();
            if (!p) return nullptr;
            return reinterpret_cast<T*>(reinterpret_cast<const Clonable*>(p)->clone());
        }

       public:
        constexpr managed_ptr() noexcept = default;

        explicit managed_ptr(T* p) noexcept
            : _ptr(p)
        {
        }

        explicit managed_ptr(std::unique_ptr<T>&& u) noexcept
            : _ptr(std::move(u))
        {
        }

        // move
        managed_ptr(managed_ptr&&) noexcept            = default;
        managed_ptr& operator=(managed_ptr&&) noexcept = default;

        // deep copy
        managed_ptr(const managed_ptr& other)
            : _ptr(clone_to_T(other.get()))
        {
        }

        managed_ptr& operator=(const managed_ptr& other)
        {
            if (this != &other) _ptr.reset(clone_to_T(other.get()));
            return *this;
        }

        T* get() const noexcept { return _ptr.get(); }

        T& operator*()
        {
            if (!get()) throw cIllegalStateExc("null");
            return *get();
        }

        const T& operator*() const
        {
            if (!get()) throw cIllegalStateExc("null");
            return *get();
        }

        T* operator->()
        {
            if (!get()) throw cIllegalStateExc("null");
            return get();
        }

        const T* operator->() const
        {
            if (!get()) throw cIllegalStateExc("null");
            return get();
        }

        explicit operator bool() const noexcept { return static_cast<bool>(_ptr); }

        void reset(T* p = nullptr) noexcept { _ptr.reset(p); }
        T* release() noexcept { return _ptr.release(); }
        void swap(managed_ptr& other) noexcept { _ptr.swap(other._ptr); }

        // deep copy
        managed_ptr duplicate() const
        {
            _enforce_clonable();
            return managed_ptr{clone_to_T(get())};
        }

        LSIZE memFootprint() const
        {
            _enforce_clonable();
            if (!get()) return static_cast<LSIZE>(sizeof(*this));
            return get()->memfp() + static_cast<LSIZE>(sizeof(*this));
        }
    };

    class Message;
    class SlotBase;
    class Socket;

    typedef managed_ptr<::crb::Message> msg_ptr;
    typedef managed_ptr<::crb::SlotBase> slot_ptr;
    typedef std::unique_ptr<::crb::Socket> cerberus_socket;

    enum DataType : uint8_t
    {
        DT_Invalid = 0,  // specified when a value has an unknown type
        DT_Integer = 2,  // false if the value contains a letter or a symbol
        DT_Double  = 3,  // false if the value does not contain a '.' or if it contains a letter
        DT_Bool    = 4,  // true only if the value equals "true" or "false" (case insensitive)
    };

    class Opaque
    {
        std::string value;

       public:
        Opaque()
            : value() {}
        CERBERUS_EXPORT Opaque(const char* str);
        CERBERUS_EXPORT Opaque(const std::string& str);
        CERBERUS_EXPORT Opaque(int val);
        CERBERUS_EXPORT Opaque(unsigned int val);
        CERBERUS_EXPORT Opaque(int64_t val);
        CERBERUS_EXPORT Opaque(uint64_t val);
        explicit CERBERUS_EXPORT Opaque(double val);
        explicit CERBERUS_EXPORT Opaque(float val);
        explicit CERBERUS_EXPORT Opaque(bool val);

        // return the contained type of data. priority: double(highest) > integer > bool > invalid
        CERBERUS_EXPORT DataType type() const;
        CERBERUS_EXPORT const std::string& get() const;

        void set(const std::string& str);  // set the string value
        void setInt(int64_t val);          // set the integer value
        void setUInt(uint64_t val);        // set the unsigned integer value
        void setDouble(double val);        // set the double value
        void setBool(bool val);            // set the bool value

        Opaque& operator=(const std::string& str);  // set the string value

        // value getters. Throw if invalid data type is requested
        CERBERUS_EXPORT int64_t getInt();
        CERBERUS_EXPORT double getDouble();
        CERBERUS_EXPORT bool getBool();
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
        OR_DuplicateKey,              // [general] duplicate primary key detected
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
        OR_DBMissingColumns,          // [database] missing required columns for operation
        OR_DBFieldSizeMismatch,       // [database] serialized field size mismatch
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

        CERBERUS_EXPORT OpRes();

        CERBERUS_EXPORT OpRes(Result r, const std::string& reason = "", const std::string& reason2 = "");

        CERBERUS_EXPORT OpRes(const OpRes& opres, const std::string& reason = "");

        bool operator==(const OpRes& other) const noexcept { return res == other.res; }
        bool operator!=(const OpRes& other) const noexcept { return res != other.res; }

        CERBERUS_EXPORT bool operator==(Result r) const;
        CERBERUS_EXPORT bool operator!=(Result r) const;

        // Throw a generic exception with the given text only if the result is failed
        CERBERUS_EXPORT OpRes& expect(const std::string& str);

        // Throw a generic exception with the given text only if the result fails and
        // only if it fails with the given reason
        CERBERUS_EXPORT OpRes& expect(Result reason, const std::string& str);

        // Throw a generic exception with the result status text only if the result fails
        CERBERUS_EXPORT OpRes& expect();

        // Return true if the Result is OR_OK, false otherwise.
        // If print is specified, the error string will be printed with the internal error info
        CERBERUS_EXPORT bool ok(const std::string& str = "");

        // Return false if the Result is OR_OK, true otherwise.
        // If print is specified, the error string will be printed with the internal error info
        CERBERUS_EXPORT bool fail(const std::string& str = "");

        // Translate the Result variable into a string
        CERBERUS_EXPORT std::string errorString() const;

        // Translate the entire Result into a string
        CERBERUS_EXPORT std::string toStr() const;

        CERBERUS_EXPORT OpRes& addOptional(Result opt);

        CERBERUS_EXPORT bool hasOptional(Result opt);

        CERBERUS_EXPORT OpRes& addInfo(const std::string& str);

        CERBERUS_EXPORT LSIZE memfp() const;
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
        }

        OpResData<T>& expect(const std::string& str)
        {
            OpRes::expect(str);
            return *this;
        }

        OpResData<T>& expect(Result reason, const std::string& str)
        {
            OpRes::expect(reason, str);
            return *this;
        }

        OpResData<T>& expect()
        {
            OpRes::expect();
            return *this;
        }

        OpResData<T>& addOptional(Result opt)
        {
            OpRes::addOptional(opt);
            return *this;
        }

        OpResData<T>& addInfo(const std::string& str)
        {
            OpRes::addInfo(str);
            return *this;
        }
    };

    class Thread;

    typedef void (*timerCallback)(void* ctx);
    typedef int (*threadTickCallback)(msg_ptr msg, Thread* thr);
    typedef void (*threadCallback)(Thread* thr);
    typedef OpRes (*playerCallback)(void* ctx, void* data);
    typedef void (*taskEndCallback)(void* ctx, void* data, OpRes result);

    struct TimerData
    {
        DateTime delay;
        TimeFrame time;
        std::atomic_bool* bit;
        std::atomic_bool* expired;
        timerCallback callback;
        void* ctx;
        HASH32 recipient;

        bool isPeriodic() { return !time.isNull(); }
        bool isDelayed() { return delay.isValid(); }
        bool isValid() { return (isPeriodic() || isDelayed()) && bit && expired; }
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
        LSIZE sz() const
        {
            return static_cast<LSIZE>(key.capacity()) + static_cast<LSIZE>(val.capacity()) +
                   static_cast<LSIZE>(sizeof(std::string) * 2);
        }
    };

    class Dictionary : public std::vector<DictLine>
    {
       public:
        // Get the value of a field as text.
        // This method will return OR_NotFound if the requested field name was not found.
        CERBERUS_EXPORT StringOpRes getFieldValue(const std::string& key,
                                                  WordMatch match = WM_CaseSensitive) const;

        // Check if a field value matches with value
        // This method will return OR_OK if the key value of the dictionary matches against
        // the provided value argument (the match policy is specified in the valmatch argument),
        // OR_NotFound if the requested field name was not found,
        // OR_Mismatch if the field name was found but it does not match with the specified value.
        CERBERUS_EXPORT OpRes getFieldMatch(const std::string& key, const std::string& value,
                                            WordMatch keymatch = WM_CaseSensitive,
                                            WordMatch valmatch = WM_CaseSensitive) const;

        // Get the name of the field at the index position.
        // An exception is thrown if index is out of bounds
        CERBERUS_EXPORT std::string getNameAt(SIZE index) const;

        // Get the value of the field at the index position.
        // An exception is thrown if index is out of bounds
        CERBERUS_EXPORT std::string getValueAt(SIZE index) const;

        // Add a key at the end of the dictionary
        CERBERUS_EXPORT Dictionary& addKey(const std::string& key, const std::string& value);

        // Get a line by reference
        // An exception is thrown if index is out of bounds
        CERBERUS_EXPORT DictLine& get(SIZE index);

        // Get a line by reference
        // An exception is thrown if key does not exist
        CERBERUS_EXPORT DictLine& get(const std::string& key, WordMatch match = WM_CaseSensitive);

        // Tell if a key is present in the dictionary
        CERBERUS_EXPORT bool exists(const std::string& key, WordMatch match = WM_CaseSensitive);

        // Print all the lines of the dictionary on a string
        CERBERUS_EXPORT std::string toString() const;

        // Return the allocated memory
        CERBERUS_EXPORT LSIZE memfp() const;
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
        CERBERUS_EXPORT Host();

        // Construct an Host with str as hostname if it contains at least one letter,
        // otherwise str will be used to extract ip:port with a stringToHost() call.
        CERBERUS_EXPORT Host(const std::string& str);

        // Same as above
        CERBERUS_EXPORT Host(const char* str);

        // Take an ip address in the form of x.x.x.x or x.x.x.x:yyyyy
        // and return an Host object with numeric IP and port members filled
        // It is possible to use "any", "local" or "broadcast" (case insensitive) in place of the
        // IP address to specify 'any interface', 'localhost' and '255.255.255.255' respectively.
        // An invalid Host is returned if the conversion fails
        CERBERUS_EXPORT static Host stringToHost(const std::string& str);

        // Build a Host from address and port without concatenating "address:port" manually.
        // Address may be numeric IPv4 or textual hostname. Port 0 yields an invalid remote host.
        CERBERUS_EXPORT static Host fromAddressPort(const std::string& address, uint16_t port);

        // Extract the port number from the given string.
        // The port number must be the last element in the string and must
        // be preceded by a column :
        // If no port is found, 0 is returned
        CERBERUS_EXPORT static uint16_t getPort(const std::string& str);

        // Extract a numeric IP address and port and saves them in this Host instance.
        // The instance becomes invalid if the conversion fails
        CERBERUS_EXPORT bool fromString(const std::string& str);

        // Print only the host part when port==0, otherwise print host:port.
        // Host part is hostname when textual, numeric IPv4 otherwise.
        CERBERUS_EXPORT std::string toString() const;

        // Print the numeric IPv4 address only (x.x.x.x), regardless of port.
        CERBERUS_EXPORT std::string ipToString() const;

        // Tells if the Host is valid
        CERBERUS_EXPORT bool isValid() const;

        // Tell if the Host is valid for remote usage, e.g. connect() or sendTo().
        // For this method to return true, the Host must have a valid port
        // and either an hostname OR a numerical IP address
        CERBERUS_EXPORT bool isValidRemote() const;

        // Tell if the Host is valid for remote usage with numeric IPv4 only.
        CERBERUS_EXPORT bool isValidNumericRemote() const;

        // Tell if the Host has a valid numerical IP
        CERBERUS_EXPORT bool isNumeric() const;

        // Tell if the Host has an hostname
        CERBERUS_EXPORT bool isTextual() const;

        // Tell if the Host has a valid port (port != 0)
        CERBERUS_EXPORT bool hasPort() const;

        // Resolve the given Host using the hostname member.
        // The resulting numeric IP address is written in the ip parameter
        CERBERUS_EXPORT OpRes resolve();
    };

    class ByteBuffer;
    class JsonData;

}  // namespace crb
#endif  // CERBERUS_TYPES_H
