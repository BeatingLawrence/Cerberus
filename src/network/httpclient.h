#ifndef CERBERUS_NETWORK_HTTPCLIENT_H
#define CERBERUS_NETWORK_HTTPCLIENT_H

#include "socket.h"
#include "src/data/httpdata.h"

namespace cerberus
{
    namespace network
    {
        class HTTPClient
        {
           private:
            std::string m_name;
            Socket *m_socket;

            bool m_useTLS;
            std::string m_certFile;
            std::string m_keyFile;

            struct DictResult
            {
                OperationResult result;
                Dictionary dict;
            };

            struct StatusResult
            {
                OperationResult result;
                data::HTTPStatus status;
            };

            DictResult getDictFromHeader(const data::ByteBuffer &header);

            StatusResult getStatus(const data::ByteBuffer &statusLine);

            void decodeChunkedData(data::ByteBuffer &data);

           public:
            HTTPClient(const std::string &name = std::string());

            ~HTTPClient();

            // Specify wether the instance should use a TLS connection or not
            void useTLS(bool use = true, const std::string &certfile = std::string(), const std::string &keyfile = std::string());

            // Connect to a remote host
            cerberus::OperationResult connectTo(const Host &host);

            // Disconnect from host
            void disconnect();

            // Perform an HTTP request using the given data
            cerberus::OperationResult makeRequest(const data::HTTPData &data);

            // Block until a response is available to be read
            cerberus::OperationResult getResponse(data::HTTPData &data, const time::TimeFrame &timeout = time::TimeFrame(1000), const time::TimeFrame &cycTimeout = time::TimeFrame());

            // Get the internal socket (use for debugging purposes)
            cerberus::network::Socket *getSocket();
        };
    }  // namespace network
}  // namespace cerberus

#endif  // CERBERUS_NETWORK_HTTPCLIENT_H
