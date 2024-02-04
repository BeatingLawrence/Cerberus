#ifndef CERBERUS_DATA_HTTPDATA_H
#define CERBERUS_DATA_HTTPDATA_H

#include "bytebuffer.h"

namespace cerberus
{
    namespace data
    {
        class HTTPData
        {
           private:
            Dictionary m_header;
            data::ByteBuffer m_payload;  // may become a vector of Bytebuffer to support multipart in the future

           protected:
            // Construct an invalid HTTPData instance
            HTTPData();

           public:
            // Add a field to the HTTP header. name parameter is case-insensitive
            HTTPData& addHeaderField(const std::string& name, const std::string& value);

            // Replace the current header dictionary with the give one
            HTTPData& setHeaderDict(const Dictionary& dict);

            // Remove a field from the HTTP header. name parameter is case-insensitive.
            // Nothing happens if the name is not found
            HTTPData& removeHeaderField(const std::string& name);

            // Set the HTTP payload
            HTTPData& setPayload(const ByteBuffer& payload);

            // Get the HTTP payload
            const data::ByteBuffer& payload() const;
            data::ByteBuffer& payload();

            // Clear all stored data
            HTTPData& clear();

            // Clear stored header fields only
            HTTPData& clearHeader();

            // Get the number of fields contained in the header
            SIZE getHeaderSize() const;

            // Get the value of a field in the str member of the result.
            // This method will return OR_NotFound if the requested field name was not found
            OperationResult getHeaderField(const std::string& key) const;

            // Get the matching result of a value corresponding to a key in the int member of the result
            // This method will return true if the key value of the header matches against the provided value argument.
            // This method will return OR_NotFound if the requested field name was not found
            OperationResult getHeaderMatch(const std::string& key, const std::string& value) const;

            // Get the name of the field at the index position.
            // An exception is thrown if index is greater or equal to the size returned by getHeaderSize()
            std::string getHeaderFieldName(SIZE index) const;

            // Get the value of the field at the index position.
            // An exception is thrown if index is greater or equal to the size returned by getHeaderSize()
            std::string getHeaderFieldValue(SIZE index) const;

            // Construct and return the header
            data::ByteBuffer getHeader() const;
        };

        class HTTPRequest : public HTTPData
        {
           public:
            HTTPRequest();

            HTTPMethod method;
            std::string url;
            HTTPVersion version;

            // Quickly fill the request data
            HTTPRequest& setup(HTTPMethod m, const std::string& u, HTTPVersion v);

            // Construct and return all the data as text(header+payload)
            data::ByteBuffer data() const;
        };

        class HTTPResponse : public HTTPData
        {
           public:
            HTTPResponse();

            HTTPVersion version;
            uint16_t statusCode;
            std::string message;

            // Quickly fill the response data
            HTTPResponse& setup(HTTPVersion v, uint16_t sc, const std::string& msg);

            // Construct and return all the data as text(header+payload)
            data::ByteBuffer data() const;

            // Check if the request has returned a good result
            bool isOk();
        };

    }  // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_HTTPDATA_H
