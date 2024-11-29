#ifndef HTTP_TCPSERVER
#define HTTP_TCPSERVER

#include <stdio.h>
#include <winsock.h>
#include <stdlib.h>
#include <string>

namespace http {
    class TcpServer {
    public:
        //const
        TcpServer(std::string ip_address, int port);
        //dest
        ~TcpServer();
        void startListen();

    private:
        std::string m_ip_address;
        std::string m_serverMessage;
        int m_port;
        int m_socketAddress_length;
        long m_incomingMessage;
        SOCKET m_socket;
        SOCKET m_newSocket;
        WSADATA m_wsaData;
        struct sockaddr_in m_socketAddress;

        int startServer();
        void closeServer();
        void acceptConnection(SOCKET &newSocket);
        std::string buildResponse();
        void sendResponse();

    };    
} //namespace http
#endif