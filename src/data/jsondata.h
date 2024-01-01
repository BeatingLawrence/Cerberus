#ifndef CERBERUS_DATA_JSONDATA_H
#define CERBERUS_DATA_JSONDATA_H

#include <string>

#include "../types.h"
#include "bytebuffer.h"
#include "filesystem/file.h"

namespace cerberus
{
    namespace data
    {
        class JsonData
        {
            enum ParseMode
            {
                None = 0,
                ParsingObject,
                ParsingArray,
            };

            std::string m_text;

            std::vector<JsonData> m_elements;

            JsonDataType m_type;

            OperationResult _parse(const ByteBuffer& buffer, ParseMode mode = None);

            void _generate(ByteBuffer& buffer);

            void _getType();

            void update();

            std::string toText();

           public:
            // Construct a null JsonData
            JsonData();

            // Construct a non-null JsonData (pure)
            JsonData(const std::string& value);
            JsonData(const char* value);
            JsonData(long double value);
            JsonData(float value);
            JsonData(bool value);

            // Construct a non-null JsonData (nested)

            JsonData(const std::string& value, const JsonData& data);
            JsonData(const char* value, const JsonData& data);

            // Iterators
            Iterator<JsonData> begin();
            Iterator<JsonData> end();
            ConstIterator<JsonData> begin() const;
            ConstIterator<JsonData> end() const;

            // Get an element of the object.
            // Use this method with index = 0 to get the value
            // if the instance is not an array or a JSON object.
            // If index is out of bounds, an exception will be thrown
            JsonData& get(SIZE index = 0);

            // Search an element of the object.
            // If no item is found, nullptr will be returned
            JsonData* search(const std::string& name);

            // Search an element in this object and all its children recursively.
            // If no item is found, nullptr will be returned.
            // NOTE: this method will return the only the first occurrence even if
            // there are more than one
            JsonData* deepSearch(const std::string& name);

            // Get the type of the object
            JsonDataType type();

            // Check if the object contains no value or null
            bool isNull();

            // Check if the object is an array
            bool isArray();

            // Check if the object is JSON object
            bool isObject();

            // Get the number of elements contained.
            // If the object is a single value, this method will return 0.
            // If the object is an array or a JSON object, this method
            // will return the number of elements contained
            SIZE size();

            // Convert the object to a numeric value
            OperationResult toNumber();

            // Convert the object to a string value
            // Initial and final " will be removed
            OperationResult toString();

            // Convert the object to a numeric value (true, false)
            OperationResult toBool();

            // Set the value of the object, discarding the previous one
            void set(const std::string& value);
            void setDouble(long double value);
            void setBool(bool value);

            JsonData& setType(JsonDataType type);

            // Add a value to the object.
            // A reference to this object is returned to allow method chaining
            JsonData& add(const JsonData& value);

            // parser section

            OperationResult parse(const ByteBuffer& buffer);

            OperationResult parse(const filesystem::File& file);

            // generator section

            OperationResult generate(ByteBuffer& buffer);

            OperationResult generate(filesystem::File& file);

            void toStr(std::string& str, uint8_t level = 0);
        };
    }  // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_JSONDATA_H
