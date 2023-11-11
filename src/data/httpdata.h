#ifndef CERBERUS_DATA_HTTPDATA_H
#define CERBERUS_DATA_HTTPDATA_H

#include "src/data/bytebuffer.h"

namespace cerberus
{
    namespace data
    {
        enum HTTPDataType
        {
            HDT_Invalid,
            HDT_Request,
            HDT_Response,
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

        struct HTTPRequest  // given to the HTTPData to make a request
        {
            HTTPMethod method;
            std::string url;
            HTTPVersion version;
        };

        struct HTTPStatus  // returned by HTTPData to tell the response status
        {
            HTTPVersion version;
            uint32_t statusCode;
            std::string message;
        };

        class HTTPData
        {
           private:
            HTTPDataType m_type;
            HTTPRequest m_request;
            HTTPStatus m_status;

            Dictionary m_header;

            data::ByteBuffer m_payload;  // may become a vector of Bytebuffer to support multipart in the future

           public:
            // Construct an invalid HTTPData instance
            HTTPData();

            // Set the instance as a request instance type and save the given request
            void setRequest(const HTTPRequest& request);

            // Set the instance as a response instance type and save the given status
            void setStatus(const HTTPStatus& status);

            // Add a field to the HTTP header. name parameter is case-insensitive
            void addHeaderField(const std::string& name, const std::string& value);

            // Remove a field from the HTTP header. name parameter is case-insensitive.
            // Nothing happens if the name is not found
            void removeHeaderField(const std::string& name);

            // Set the HTTP payload
            void setPayload(const ByteBuffer& payload);

            // Clear this HTTPData instance completely
            void clear();

            // Tell if the instance is valid and contains valid data
            bool isValid() const;

            // Tell if the instance is an HTTP request
            bool isRequest() const;

            // Tell if the instance is an HTTP response
            bool isResponse() const;

            // Get the request line data
            HTTPRequest getRequest() const;

            // Get the status line data
            HTTPStatus getStatus() const;

            // Get the number of fields contained in the header
            SIZE getHeaderSize() const;

            // Get the value of a field in the str member of the result.
            // This method will return OR_NotFound if the requested field name was not found
            OperationResult getHeaderField(const std::string& key) const;

            // Get the matching result of a value corresponding to a key in the int member of the result
            // This method will return 1 if the key value of the header matches against the provided value argument.
            // This method will return OR_NotFound if the requested field name was not found
            OperationResult getHeaderMatch(const std::string& key, const std::string& value) const;

            // Get the name of the field at the index position.
            // An exception is thrown if index is greater or equal to the size returned by getHeaderSize()
            std::string getHeaderFieldName(SIZE index) const;

            // Get the value of the field at the index position.
            // An exception is thrown if index is greater or equal to the size returned by getHeaderSize()
            std::string getHeaderFieldValue(SIZE index) const;

            // Get the HTTP payload
            const data::ByteBuffer& getPayload() const;
            data::ByteBuffer& getPayload();
        };
    }  // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_HTTPDATA_H
