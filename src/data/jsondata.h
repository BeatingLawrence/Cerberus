#ifndef CERBERUS_DATA_JSONDATA_H
#define CERBERUS_DATA_JSONDATA_H

#include <string>

#include "../types.h"
#include "bytebuffer.h"
#include "filesystem/file.h"

namespace cerberus
{
    class JsonData
    {
        enum ParseMode
        {
            Unspecified = 0,
            ParsingObject,
            ParsingArray,
        };

        enum Integrity
        {
            None,
            Degraded,
            GoodArray,
            GoodObject,
        };

        std::string m_name, m_value;

        std::vector<JsonData> m_elements;

        JsonDataType m_type;

        BoolOpRes _parse(const ByteBuffer& buffer, ParseMode mode = Unspecified);

        void _generate(ByteBuffer& buffer) const;

        Integrity checkIntegrity() const;

       public:
        // Construct an invalid JsonData
        JsonData();

        // Construct a valid JsonData that contains a Json null
        JsonData(const std::string& name);

        // Construct a valid JsonData that contains a Json string
        JsonData(const std::string& name, const std::string& value);
        JsonData(const std::string& name, const char* value);

        // Construct a valid JsonData that contains a Json number
        JsonData(const std::string& name, long double value);
        JsonData(const std::string& name, float value);
        JsonData(const std::string& name, int64_t value);

        // Construct a valid JsonData that contains a Json boolean
        JsonData(const std::string& name, bool value);

        // Construct a valid JsonData object that contains another JsonData (nesting)
        JsonData(const std::string& name, const JsonData& data);

        // Iterators
        Iterator<JsonData> begin();
        Iterator<JsonData> end();
        ConstIterator<JsonData> begin() const;
        ConstIterator<JsonData> end() const;

        // Get an element of the object.
        // If index is out of bounds, an invalid JsonData will be returned
        JsonData getAt(SIZE index = 0);

        // Get an element of the object.
        // If no item is found, an invalid JsonData will be returned
        JsonData get(const std::string& name);

        // Search an element in this object and all its children recursively.
        // If no item is found, an invalid JsonData will be returned.
        // NOTE: this method will return only the first occurrence even if
        // there are more than one
        JsonData search(const std::string& name);

        // Get the type of the object
        JsonDataType type() const;

        // Check if the object is valid
        bool isValid() const;

        // Check if the object is a Json null
        bool isNull() const;

        // Check if the object is a Json number
        bool isNumber() const;

        // Check if the object is a Json string
        bool isString() const;

        // Check if the object is a Json boolean
        bool isBoolean() const;

        // Check if the object is a Json array
        bool isArray() const;

        // Check if the object is a Json object
        bool isObject() const;

        // Get the number of elements contained.
        // If the object is a single value, this method will return 0.
        // If the object is an array or a JSON object, this method
        // will return the number of elements contained
        SIZE size() const;

        // Check if size() == 0
        bool empty() const;

        // Recursively check if the data contained inside the instance is good
        bool check() const;

        // Recursively check if the data contained inside the instance is good
        // Also try to set correct types (only for array and object) if possible
        OpRes checkFix();

        // Convert the object to a floating point numeric value
        FloatOpRes toNumber() const;

        // Convert the object to an integer numeric value
        IntOpRes toIntNumber() const;

        // Convert the object to a string value
        // The string will not contain initial and final "
        StringOpRes toString() const;

        // Convert the object to a bool value (true, false)
        BoolOpRes toBool() const;

        // Set the name of the object
        JsonData& setName(const std::string& name);

        // Set the value of the object, discarding the previous one
        JsonData& setString(const std::string& value);
        JsonData& setNumber(long double value);
        JsonData& setNumber(float value);
        JsonData& setNumber(int64_t value);
        JsonData& setBoolean(bool value);

        // Add an object to this object.
        JsonData& add(const JsonData& object);

        // Add an unnamed value to the object.
        // This methods may change the type
        JsonData& add(const std::string& value);
        JsonData& add(const char* value);
        JsonData& add(long double value);
        JsonData& add(float value);
        JsonData& add(int64_t value);
        JsonData& add(bool value);

        // Remove all the nested object only
        JsonData& clear();

        // Parse the given ByteBuffer and fill this instance using the obtained data
        OpRes parse(const ByteBuffer& buffer);

        // Parse the given File content and fill this instance using the obtained data
        OpRes parse(const File& file);

        // Generate the instance using the JSON syntax
        OpResData<ByteBuffer> generate() const;

        // Write the generated data to the given file. The File must be open
        OpRes generate(File& file) const;

        // Print the content using a debug formatting rule
        void toStr(std::string& str, uint8_t level = 0) const;

        // Calculate the memory footprint
        SIZE memfp() const;
    };
}  // namespace cerberus

#endif  // CERBERUS_DATA_JSONDATA_H
