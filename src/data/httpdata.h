#ifndef CERBERUS_DATA_HTTPDATA_H
#define CERBERUS_DATA_HTTPDATA_H

#include "bytebuffer.h"
#include "jsondata.h"

namespace cerberus
{
    class HTTPData
    {
       private:
        Dictionary m_header;

        // may become a vector of Bytebuffer to support multipart in the future
        ByteBuffer m_payload;

        void setPayloadSize(SIZE s);

       protected:
        // Construct an empty HTTPData instance
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

        // Perform a JSON generation and set the payload to the generated text.
        // The result of the generation will be returned.
        // If the generation fails, this instance will contain inconsistent data,
        // so it's recommended not to use it in a HTTP request
        OpRes setJsonPayload(const JsonData& payload);

        // Get the HTTP payload
        const ByteBuffer& payload() const;
        ByteBuffer& payload();

        // Parse the payload to a JsonData object and return it
        OpResData<JsonData> JsonPayload() const;

        // Clear all stored data
        HTTPData& clear();

        // Clear stored header fields only
        HTTPData& clearHeader();

        // Get the number of fields contained in the header
        SIZE getHeaderSize() const;

        // Get the value of a field in the str member of the result.
        // This method will return OR_NotFound if the requested field name was not found.
        // The search is case insensitive.
        StringOpRes getHeaderField(const std::string& key) const;

        // Get the matching result of a value corresponding to a key in the int member of the result
        // This method will return OR_OK if the key value of the header matches against
        // the provided value argument (case sensitive),
        // OR_NotFound if the requested field name was not found,
        // OR_Mismatch if the field name was found but it does not match with the specified value.
        OpRes getHeaderMatch(const std::string& key, const std::string& value) const;

        // Get the name of the field at the index position.
        // An exception is thrown if index is greater or equal to the size returned by getHeaderSize()
        std::string getHeaderFieldName(SIZE index) const;

        // Get the value of the field at the index position.
        // An exception is thrown if index is greater or equal to the size returned by getHeaderSize()
        std::string getHeaderFieldValue(SIZE index) const;

        // Construct and return the header
        ByteBuffer getHeader() const;
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
        ByteBuffer data() const;
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
        ByteBuffer data() const;

        // Check if the request has returned a non-error result
        bool isOk();
    };
}  // namespace cerberus

#endif  // CERBERUS_DATA_HTTPDATA_H
