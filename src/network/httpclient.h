#ifndef CERBERUS_NETWORK_HTTPCLIENT_H
#define CERBERUS_NETWORK_HTTPCLIENT_H

#include "../data/httpdata.h"
#include "socket.h"

namespace cerberus
{
    namespace network
    {
        class HTTPClient
        {
           private:
            Socket m_socket;

            static OpRes getDictFromHeader(const data::ByteBuffer &header, Dictionary &dict);

            static OpRes getStatus(const data::ByteBuffer &statusLine, data::HTTPResponse &response);

            static void decodeChunkedData(data::ByteBuffer &data);

           public:
            HTTPClient(const std::string &name = std::string());

            ~HTTPClient();

            // Set the socket to be a TLS socket
            OpRes TLS_init(const std::string &certfile = "", const std::string &keyfile = "");

            // Free all the allocated resources for the TLS features, thus, a call to initTLS() is necessary
            // for the socket to send and receive on the secure layer again.
            OpRes TLS_deinit();

            // Connect to a remote host
            cerberus::OpRes connectTo(const Host &host);

            // Disconnect from host
            void disconnect();

            // Perform an HTTP request using the given data
            cerberus::OpRes makeRequest(const data::HTTPRequest &data);

            // Block until a response is available to be read
            cerberus::OpResData<data::HTTPResponse> getResponse(const time::TimeFrame &timeout = time::TimeFrame(1000), const time::TimeFrame &cycTimeout = time::TimeFrame());

            // Get the internal socket (use for debugging purposes)
            cerberus::network::Socket *getSocket();
        };
    }  // namespace network
}  // namespace cerberus

#endif  // CERBERUS_NETWORK_HTTPCLIENT_H
