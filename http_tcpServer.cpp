#include "http_tcpServer.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

namespace {
    const int BUFFER_SIZE = 30720;

    void log(const std::string &msg) {
        std::cout << msg << std::endl;
    }

    void exitWithError(const std::string &msg) {
        std::cout << WSAGetLastError() << std::endl;
        log("Error: " + msg);
        exit(1);
    }
}

namespace http {
    TcpServer::TcpServer(std::string ip_address, int port) :
                                                                m_ip_address(ip_address),
                                                                m_port(port),
                                                                m_socket(),
                                                                m_newSocket(),
                                                                m_incomingMessage(),
                                                                m_socketAddress(),
                                                                m_socketAddress_length(sizeof(m_socketAddress)),
                                                                m_serverMessage(buildResponse()),
                                                                m_wsaData() {
       m_socketAddress.sin_family = AF_INET;
       m_socketAddress.sin_port = htons(m_port);
       m_socketAddress.sin_addr.s_addr = inet_addr(m_ip_address.c_str());

       if(startServer() != 0) {
        std::ostringstream stringStream;
        stringStream << "Failure to start server with PORT: " << ntohs(m_socketAddress.sin_port);
        log(stringStream.str());
       } 
    }
    TcpServer::~TcpServer() {
        closeServer();
    }

    int TcpServer::startServer() {
        if (WSAStartup(MAKEWORD(2, 0), &m_wsaData) != 0) {
            exitWithError("WSAStartup failed.");
        }
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0) {
            exitWithError("Cannot create socket.");
            return 1;
        }
        if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_length) < 0) {
            std::stringstream stringStream;
            stringStream << "Cannot bind to SOCKET ADDRESS: " << &m_socketAddress;
            exitWithError(stringStream.str());
            return 1;
        }
        return 0;
    }

    void TcpServer::closeServer() {
        closesocket(m_socket);
        closesocket(m_newSocket);
        WSACleanup();
        exit(0);
    }

    void TcpServer::startListen() {
        if (listen(m_socket, 20) < 0) {
            exitWithError("Failed to listen to socket.");
        }

        std::ostringstream stringStream;
        stringStream << "\n Listening on ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << "\n PORT: " << ntohs(m_socketAddress.sin_port) << "\n";
        log(stringStream.str());

        int bytesRcvd = 0;
        log("Waiting for connection...\n");

        while (m_newSocket != 0) {
            acceptConnection(m_newSocket);

            char buf[BUFFER_SIZE] = {0};
            bytesRcvd = recv(m_newSocket, buf, BUFFER_SIZE, 0);
            if (bytesRcvd < 0) {
                exitWithError("Failure to receive bytes from client socket connection.");
            }

            stringStream << "Received request from Client\n";
            log(stringStream.str());

            sendResponse();
            closesocket(m_newSocket);
        }
    }

    void TcpServer::acceptConnection(SOCKET &new_socket) {
        new_socket = accept(m_socket, (sockaddr *)&m_socketAddress, &m_socketAddress_length);
        if (new_socket < 0) {
            std::ostringstream stringStream;
            stringStream << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << ", PORT: "  << ntohs(m_socketAddress.sin_port);
            exitWithError(stringStream.str());
        }
    }

    std::string TcpServer::buildResponse() {
        std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from the Server </p></body></html>";
        std::ostringstream stringStream;
        stringStream << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n" << htmlFile;
        return stringStream.str();
    }

    void TcpServer::sendResponse() {
        int bytesSent;
        long totalBytesSent = 0;

        while (totalBytesSent < m_serverMessage.size()) {
            bytesSent = send(m_newSocket, m_serverMessage.c_str(), m_serverMessage.size(), 0);
            if (bytesSent < 0) {
                break;
            }
            totalBytesSent += bytesSent;
        }

        if(totalBytesSent == m_serverMessage.size()) {
            log("Server Response sent to Client.\n");
        } else {
            log("Error sending response to Client.\n");
        }
    }
} //namespace http