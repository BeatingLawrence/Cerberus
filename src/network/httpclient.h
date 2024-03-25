#ifndef CERBERUS_NETWORK_HTTPCLIENT_H
#define CERBERUS_NETWORK_HTTPCLIENT_H

#include "../data/httpdata.h"
#include "socket.h"

namespace cerberus
{
    class HTTPClient : private Socket
    {
       private:
        bool m_persistent;

        Host m_remote;

        cerberus::OpRes _connect();

        static OpRes getDictFromHeader(const ByteBuffer &header, Dictionary &dict);

        static OpRes getStatus(const ByteBuffer &statusLine, HTTPResponse &response);

        static void decodeChunkedData(ByteBuffer &data);

       public:
        HTTPClient(const std::string &name = std::string());

        virtual ~HTTPClient();

        // using Socket::close;
        using Socket::TLS_deinit;
        using Socket::TLS_ignoreHangup;
        using Socket::TLS_init;

        void setRemote(const Host &host);

        // Make the client persistent.
        // A persistent client automatically reconnects to the server if the connection
        // drops, when the application makes a request.
        void persistent(bool persistent = true);

        // Connect to the remote (if not already connected) and
        // send an HTTP request using the given data
        cerberus::OpRes makeRequest(const HTTPRequest &request);

        // Block until a response is available to be read, then, if connection is
        // not persistent, close the socket
        cerberus::OpResData<HTTPResponse> getResponse(const TimeFrame &timeout    = TimeFrame(1000),
                                                      const TimeFrame &cycTimeout = TimeFrame());

        // Get HTTP data. This method is a combination of makeRequest and getResponse
        cerberus::OpResData<HTTPResponse> get(const HTTPRequest &request,
                                              const TimeFrame &timeout    = TimeFrame(1000),
                                              const TimeFrame &cycTimeout = TimeFrame());
    };
}  // namespace cerberus

#endif  // CERBERUS_NETWORK_HTTPCLIENT_H
