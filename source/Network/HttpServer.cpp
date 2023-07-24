
#include "HttpServer.h"

#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <fcntl.h> 

#include <unistd.h>
#include <cstdint>

#include <netinet/in.h>

#include "../Log.h"
#include "../SavefileIO.h"

HttpServer::HttpServer() {};

void HttpServer::Start(int port)
{
    m_Thread = std::thread( [&] { this->ThreadEntryPoint(port); } );
}

void HttpServer::Stop()
{
    m_IsRunning = false;
    Log("Joining http thread");
    m_Thread.join();
    Log("Joined");
}

void HttpServer::ThreadEntryPoint(int port)
{
    // Check if online
    u32 ip = gethostid();
    if (ip == INADDR_LOOPBACK) // Offline
        return;

    // Create socket
    m_SocketDescriptor = socket(AF_INET , SOCK_STREAM , 0);
    if (m_SocketDescriptor == -1)
    {
        printf("Could not create socket");
    }

    // Make the socket calls non-blocking
    // https://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking
    if (fcntl(m_SocketDescriptor, F_SETFL, fcntl(m_SocketDescriptor, F_GETFL, 0) | O_NONBLOCK) == -1){
        Log("Error calling fcntl");
        // handle the error.  By the way, I've never seen fcntl fail in this way
    }
        
    // Prepare the sockaddr_in structure
    m_Server.sin_family = AF_INET; // TCP
    m_Server.sin_addr.s_addr = INADDR_ANY;
    m_Server.sin_port = htons(port);
        
    // Bind
    if (bind(m_SocketDescriptor, (struct sockaddr*)&m_Server , sizeof(m_Server)) < 0)
    {
        Log("Socket bind failed");
    }
    Log("Socket bind done");

    m_IsRunning = true;

    while (m_IsRunning)
    {
        bool success = WaitForClient();

        if (!success) continue;

        WaitForMessage();
    }
}

bool HttpServer::WaitForClient()
{
    //Listen
	listen(m_SocketDescriptor, 3);
	
	//Accept and incoming connection
	Log("Waiting for incoming connections...");
	int adressLength = sizeof(struct sockaddr_in); // ?
    while (m_IsRunning)
    {
        m_ClientSocket = accept(m_SocketDescriptor, (struct sockaddr *)&m_Client, (socklen_t*)&adressLength);
        if (m_ClientSocket < 0)
        {
            if (errno != EWOULDBLOCK)
                Log("Accept failed");

            // No connection. Keep polling
        }   
        else
        {
            // Connection
            Log("Connection accepted");
            return true;
        }    
    }

    return false;
}

bool HttpServer::WaitForMessage()
{
    //Receive a message from client
    int readSize = 0;
    const int messageBufferLength = 2000; // 2000 is hopefully large enough :)
    char messageBuffer[messageBufferLength];

    int totalRead = 0;

    while (m_IsRunning)
    {
        readSize = recv(m_ClientSocket, messageBuffer, messageBufferLength, 0);
        if (readSize < 0)
        {
            if (errno != EWOULDBLOCK)
                Log("recv failed");
            
            continue;
        }

        if (readSize == 0)
        {
            Log("Connection closed");
            return false;
        }

        Log(readSize);

        totalRead += readSize;

        if (totalRead < 20) continue; // Only the first 8 chars are read, so this is enough for this purpose

        Log("Got message");
        
        printf("response, %s\n", messageBuffer);

        char indexChar = messageBuffer[8];
        if (!isdigit(indexChar))
        {
            Log("Invalid request");
            return false;
        }

        int index = indexChar - '0';

        // Recieved message
        std::string imageFilepath = "sdmc:/switch/totk-unexplored/map" + std::to_string(index) + ".png";
        Log("Loading image " + imageFilepath);
        if (!SavefileIO::Get().FileExists(imageFilepath))
        {
            Log("File doesnt exist");
            close(m_ClientSocket);

            return false;
        }    

        std::ifstream file;
        file.open(imageFilepath, std::ios::binary);

        // Get length of file
        file.seekg(0, file.end);
        unsigned int fileSize = (unsigned int)file.tellg(); // Get file size
        file.seekg(0, file.beg);

        unsigned char* imageBuffer = new unsigned char[fileSize];

        // Read the entire file into the buffer. Need to cast the buffer to a non-signed char*.
        file.read((char *)&imageBuffer[0], fileSize);

        file.close();

        //Send the message back to client
        std::string headers = "HTTP/1.1 200 OK\r\n";
        headers += "Content-Type: image/png\r\n";
        headers += "Content-Length: " + std::to_string(fileSize) + "\r\n";
        headers += "Server: Totk-Unexplored (Nintendo Switch)\r\n";
        headers += "\r\n";

        if (SendString(m_ClientSocket, headers) == -1)
        {
            Log("Error sending image");
            close(m_ClientSocket);

            return false;
        } else 
        {
            while (m_IsRunning)
            {
                Log("Sending response");

                // If SendData return -1, error. 
                // When it returns all data has been sent.
                if (SendData(m_ClientSocket, imageBuffer, fileSize) == -1)
                {
                    close(m_ClientSocket);
                    return false;
                }

                close(m_ClientSocket);
                return true;
            }
        }

        close(m_ClientSocket);

        return true;
    }

    return false;
}

int HttpServer::SendData(int socket, const void* data, int dataLength)
{
    const unsigned char *ptr = static_cast<const unsigned char*>(data);
    while (dataLength > 0) {
        int bytes = send(socket, ptr, dataLength, 0);
        if (bytes == -1)
        {
            if (errno == EWOULDBLOCK) 
                continue;
            
            return -1;
        }

        ptr += bytes;
        dataLength -= bytes;
    }

    return 0;
}

int HttpServer::SendString(int socket, const std::string& s)
{
    return SendData(socket, s.c_str(), s.size());
}

HttpServer::~HttpServer() {};