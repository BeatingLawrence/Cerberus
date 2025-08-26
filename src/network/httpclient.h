#ifndef CERBERUS_HTTPCLIENT_H
#define CERBERUS_HTTPCLIENT_H

#include "../data/httpdata.h"
#include "socket.h"

namespace cerberus
{
    class HTTPClient : private Socket
    {
       private:
        bool m_persistent;

        Host m_remote;

        HTTPResponse m_currentStreamResponse;

        cerberus::OpRes _connect();

        OpResData<HTTPResponse> _parseResponseHeader(const ByteBuffer &buf);

        static OpRes getDictFromHeader(const ByteBuffer &header, Dictionary &dict);

        static OpRes getStatus(const ByteBuffer &statusLine, HTTPResponse &response);

        static void decodeChunkedData(ByteBuffer &data);

       public:
        // construct a client using a default receive buffer size of 8K
        HTTPClient(size_t bufsize = MEM_8K);

        virtual ~HTTPClient();

        // using Socket::close;
        using Socket::TLS_deinit;
        using Socket::TLS_ignoreHangup;
        using Socket::TLS_init;

        void setRemote(const Host &host);

        // Make the client persistent.
        // A persistent client automatically reconnects to the server if the connection
        // drops, when the application makes a request. Also, a persistent client does not
        // close the connection after a response has been retrieved
        void persistent(bool persistent = true);

        // Connect to the remote (if not already connected) and
        // send an HTTP request using the given data
        cerberus::OpRes makeRequest(const HTTPRequest &request);

        // Block until a response is available to be read, then, if connection is
        // not persistent, close the socket
        cerberus::OpResData<HTTPResponse> getResponse(const TimeFrame &timeout = TimeFrame(1000),
                                                      const TimeFrame &cyc     = TimeFrame());

        // If the server never stops to send data, the getStream() method must be used instead.
        // This version downloads the payload partially (up to buffer size) and then returns.
        // The application must keep calling this method to get the stream data.
        // NOTE: all the calls will return the same HTTPResponse data, but with different payloads.
        cerberus::OpResData<HTTPResponse> getStream(const TimeFrame &timeout = TimeFrame(1000));

        // Get HTTP data. This method is a combination of makeRequest and getResponse
        cerberus::OpResData<HTTPResponse> get(const HTTPRequest &request,
                                              const TimeFrame &timeout    = TimeFrame(1000),
                                              const TimeFrame &cycTimeout = TimeFrame());

        // Force client connection close
        OpRes disconnect();
    };
}  // namespace cerberus

#endif  // CERBERUS_HTTPCLIENT_H
