#pragma once

#include <arpa/inet.h> //inet_addr
#include <string>
#include <thread>

class HttpServer
{
public:
    HttpServer();

    void Start(int port);
    
    void Stop();

    void ThreadEntryPoint(int port);

    ~HttpServer();

private:

    bool WaitForClient();
    bool WaitForMessage();

    int SendData(int socket, const void* data, int dataLength);
    int SendString(int socket, const std::string& s);

private:
    struct sockaddr_in m_Server;
    struct sockaddr_in m_Client;

    int m_SocketDescriptor;
    int m_ClientSocket;

    bool m_IsRunning = false;

    std::thread m_Thread;
};