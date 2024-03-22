#ifndef TYPES_H
#define TYPES_H

#ifndef WINDOWS_SYSTEM
#include <sys/stat.h>
#endif

#include <cstdint>
#include <list>
#include <string>
#include <type_traits>
#include <vector>

#include "mutex/mutexlocker.h"
#include "time/datetime.h"

#define StringOpRes ::cerberus::OpResData<std::string>
#define BoolOpRes ::cerberus::OpResData<bool>
#define IntOpRes ::cerberus::OpResData<int64_t>
#define FloatOpRes ::cerberus::OpResData<long double>
#define SizeOpRes ::cerberus::OpResData<::cerberus::LSIZE>

namespace cerberus
{
    typedef uint32_t SIZE;
    typedef uint64_t LSIZE;
    typedef uint8_t BYTE;
    typedef int64_t OFFSET;
    typedef uint32_t HASH32;

    struct Path : private std::list<std::string>
    {
        Path() = default;

        Path(const std::string& path) { fromStr(path); }
        Path(const char* path) { fromStr(path); }

        using std::list<std::string>::back;
        using std::list<std::string>::empty;

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

            for (auto el = begin(), en = --end(); el != en; el++)
            {
                ret.append((*el));
                ret.append("/");
            }

            ret.append(back());

            return ret;
        }

        void fromStr(const std::string& str)
        {
            clear();
            std::string tmp;

            for (auto& el : str)
            {
                if (el == '/')
                {
                    if (!tmp.empty()) push_back(tmp);  // TODO USE REGEX
                    tmp.clear();
                }
                else
                {
                    tmp.push_back(el);
                }
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

    struct CerbVersion
    {
        enum VersionType
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

    enum FileType
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

        void fromStat(const struct stat& stat_struct);
    };

    enum SlotType
    {
        ST_UNDEFINED = 0,
        ST_BYTE,
        ST_INT32,
        ST_INT64,
        ST_UINT64,
        ST_FLOAT,
        ST_DOUBLE,
        ST_BOOL,
        ST_VOIDP,
        ST_STRING,
        ST_BYTEBUFFER,
        ST_DICTIONARY,
        ST_JSON,
        ST_HOST,
        ST_TASK,
        ST_RESULT,
    };

    struct SlotTemplate
    {
        SlotType type;
        std::string name;
    };

    enum WordMatch
    {
        WM_CaseSensitive,
        WM_CaseInsensitive,
    };

    enum FileOpenMode
    {
        FOM_Read = 0,         // Open the file for reading only; the file must exist
        FOM_ReadWrite,        // Open the file for reading and writing; the file must exist
        FOM_ReadWriteTrunc,   // Open the file for reading and writing; if the file exists the
                              // content is discarded, otherwise, the file is created
        FOM_ReadWriteAppend,  // Open the file for reading and writing; if the file does not esist,
                              // it is created. All the write operations happen at the end of the
                              // file
    };

    enum ThreadPeriodicity
    {
        TP_Message,
        TP_Periodic,
        TP_PeriodicMessage,
        TP_OneShot,
        TP_Continuos,
        TP_Trigger,
    };

    enum Radix
    {
        Decimal,
        Hexadecimal,
        Binary,
    };

    enum LogLevel
    {
        LL_Info    = 0,
        LL_Warning = 1,
        LL_Error   = 2,
        LL_Debug   = 3,
    };

    enum SSLShutdownState
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
    };

    struct CerberusInitConf
    {
        LogConf logSetup;
        CoreConf coreSetup;
        bool useCiphers;  // enable cerberus to init and use the OpenSSL library
    };

    enum HTTPVersion
    {
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2,
    };

    enum HTTPMethod
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
        virtual Clonable* clone() const = 0;
    };

    template <typename T>
    class managed_ptr
    {
        Clonable* m_ptr;
        size_t* m_refcount;

        managed_ptr(T* ptr, size_t* refcount)
            : m_ptr(ptr),
              m_refcount(refcount){};

        void _destroy()
        {
            if (!m_refcount) return;

            if ((*m_refcount) == 1)
            {
                delete m_ptr;
                delete m_refcount;
            }
            else
                (*m_refcount)--;

            m_ptr      = nullptr;
            m_refcount = nullptr;
        };

       public:
        managed_ptr()
            : m_ptr(nullptr),
              m_refcount(nullptr){};

        managed_ptr(T* ptr)
            : m_ptr((Clonable*)ptr),
              m_refcount(nullptr)
        {
            if (m_ptr == nullptr) throw std::exception();

            // T must inherit Clonable
            static_assert(std::is_convertible<T*, Clonable*>());

            m_refcount = new size_t(1);
        };

        // copy constructor make a deep copy (safest)
        managed_ptr(const managed_ptr& other)
            : m_ptr((T*)(other.m_ptr->clone())),
              m_refcount(new size_t(1))
        {
            (*m_refcount)++;
        };

        managed_ptr(managed_ptr&& other)
            : m_ptr(other.m_ptr),
              m_refcount(other.m_refcount)
        {
            other.m_ptr      = nullptr;
            other.m_refcount = nullptr;
        };

        ~managed_ptr() { _destroy(); };

        // duplicate the data (deep copy)
        managed_ptr<T> duplicate() { return managed_ptr<T>((T*)(m_ptr->clone())); };

        // just increment ref counter
        managed_ptr<T> ref()
        {
            (*m_refcount)++;
            return managed_ptr<T>((T*)m_ptr, m_refcount);
        };

        T& operator*() const { return *((T*)m_ptr); };

        T* operator->() const { return (T*)m_ptr; };

        T* get() const { return (T*)m_ptr; };

        // shared copy
        managed_ptr<T>& operator=(const managed_ptr<T>& other)
        {
            _destroy();
            m_ptr      = other.m_ptr;
            m_refcount = other.m_refcount;
            (*m_refcount)++;
            return *this;
        };
    };

    namespace message
    {
        class Message;
    }

    class BaseSlot;

    typedef managed_ptr<class cerberus::message::Message> cerberus_message;
    typedef managed_ptr<const class cerberus::message::Message> cerberus_const_message;
    typedef managed_ptr<class cerberus::BaseSlot> slot_ptr;

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
        OR_TemporaryUnavailable,      // [general] the requested operation is not available at the
                                      // moment, retry later
        OR_InvalidFile,               // [general] the provided file instance is not valid
        OR_Duplicate,                 // [general] the item is a duplicate
        OR_WrongType,                 // [general] the item type is wrong
        OR_NotEmpty,                  // [general] the item is not empty
        OR_Empty,                     // [general] the item is empty
        OR_Mismatch,                  // [general] the item does not match
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
        OR_TableAlreadyPresent,       // [database] the table is already present
                                      //
        OR_ThreadNotJoinable,         // [thread] the thread is not joinable
    };

    enum MutexType : uint8_t
    {
        Simple,     // ERRORCHECK mutex
        Recursive,  // RECURSIVE mutex
    };

    // The basic Operation Result object contains a Result member and some data.
    struct OpRes
    {
        Result res;

        std::string reason;

        std::vector<Result> optional;

        OpRes();

        OpRes(Result r, const std::string& reason = "", const std::string& reason2 = "");

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

        // Translate the Result
        std::string errorString();

        OpRes& addOptional(Result opt);

        bool hasOptional(Result opt);
    };

    // The OpResData template class is useful to exchange some custom data alongside with the result
    template <class T>
    struct OpResData : OpRes
    {
        T value;

        OpResData(Result r, const std::string& reason = "", const std::string& reason2 = "")
            : OpRes(r, reason, reason2),
              value(){};

        OpResData(const T& value)
            : OpRes(OR_OK),
              value(value){};

        OpResData(const OpRes& res)
            : OpRes(res),
              value(){};

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

    class Actor;
    class Thread;

    typedef void (*timerCallback)();
    typedef int (*threadTickCallback)(cerberus_message, Thread*);
    typedef void (*threadCallback)();
    typedef OpRes (*actorCallback)(void* ctx);
    typedef void (*taskEndCallback)(void* ctx, Actor*, OpRes);

    struct Task
    {
        actorCallback cb;
        void* ctx;

        bool isValid() { return cb; };
    };

    struct DictLine
    {
        std::string key, val;
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

    namespace data
    {
        class ByteBuffer;
        class JsonData;
    }  // namespace data

    struct TypeWrapper
    {
        SlotType type;
        union
        {
            BYTE _byte;
            int32_t _int32;
            int64_t _int64;
            float _float;
            double _double;
            bool _bool;
            void* _voidp;
        };

        TypeWrapper(BYTE v)
            : type(ST_BYTE),
              _byte(v){};

        TypeWrapper(int32_t v)
            : type(ST_INT32),
              _int32(v){};

        TypeWrapper(int64_t v)
            : type(ST_INT64),
              _int64(v){};

        TypeWrapper(float v)
            : type(ST_FLOAT),
              _float(v){};

        TypeWrapper(double v)
            : type(ST_DOUBLE),
              _double(v){};

        TypeWrapper(bool v)
            : type(ST_BOOL),
              _bool(v){};

        TypeWrapper(void* v)
            : type(ST_VOIDP),
              _voidp(v){};

        TypeWrapper(std::string& v)
            : type(ST_STRING),
              _voidp(&v){};

        TypeWrapper(data::ByteBuffer& v)
            : type(ST_BYTEBUFFER),
              _voidp(&v){};

        TypeWrapper(Dictionary& v)
            : type(ST_DICTIONARY),
              _voidp(&v){};

        TypeWrapper(data::JsonData& v)
            : type(ST_JSON),
              _voidp(&v){};
    };

}  // namespace cerberus
#endif  // TYPES_H
