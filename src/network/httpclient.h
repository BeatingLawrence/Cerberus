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

            bool m_persistent;

            Host m_server;

            cerberus::OpRes _connect();

            static OpRes getDictFromHeader(const data::ByteBuffer &header, Dictionary &dict);

            static OpRes getStatus(const data::ByteBuffer &statusLine, data::HTTPResponse &response);

            static void decodeChunkedData(data::ByteBuffer &data);

           public:
            HTTPClient(const std::string &name = std::string());

            ~HTTPClient();

            // Set the socket as a TLS socket
            OpRes TLS_init(const std::string &ca_file = "", const std::string &certfile = "",
                           const std::string &keyfile = "");

            OpRes TLS_ignoreHangup(bool ignore = true);

            // Free all the allocated resources for the TLS features, thus, a call to initTLS() is
            // necessary for the socket to send and receive on the secure layer again.
            OpRes TLS_deinit();

            // Connect to a remote host
            cerberus::OpRes connect(const Host &host);

            // Disconnect from host
            void disconnect();

            // Make the client persistent.
            // A persistent client automatically reconnects to the server if the connection
            // drops, when the application makes a request.
            void persistent(bool persistent = true);

            // Perform an HTTP request using the given data
            cerberus::OpRes makeRequest(const data::HTTPRequest &request);

            // Block until a response is available to be read
            cerberus::OpResData<data::HTTPResponse> getResponse(
                const time::TimeFrame &timeout    = time::TimeFrame(1000),
                const time::TimeFrame &cycTimeout = time::TimeFrame());

            // Get HTTP data. This method is a combination of makeRequest and getResponse
            cerberus::OpResData<data::HTTPResponse> get(
                const data::HTTPRequest &request, const time::TimeFrame &timeout = time::TimeFrame(1000),
                const time::TimeFrame &cycTimeout = time::TimeFrame());

            // Get the internal socket (use for debugging purposes)
            cerberus::network::Socket &getSocket();
        };
    }  // namespace network
}  // namespace cerberus

#endif  // CERBERUS_NETWORK_HTTPCLIENT_H
