/*
	TCP/IP client
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"

#define MBtoByte 1048576 // 1048576 -> 1MB TO 1048576 Bytes

int main()
{
    char buff[1024];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            int sock = socket(AF_INET, SOCK_STREAM, 0);

            if (sock == -1)
            {
                printf("Could not create socket : %d", errno);
            }

            socklen_t len = sizeof(buff);
            if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buff, &len) != 0)
            {
                perror("getsockopt");
                return -1;
            }

            strcpy(buff, i > 0 ? "reno" : "cubic");

            len = strlen(buff);
            if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buff, len) != 0)
            {
                perror("setsockopt");
                return -1;
            }
            len = sizeof(buff);
            if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, buff, &len) != 0)
            {
                perror("getsockopt");
                return -1;
            }

            // "sockaddr_in" is the "derived" from sockaddr structure
            // used for IPv4 communication. For IPv6, use sockaddr_in6
            //
            struct sockaddr_in serverAddress;
            memset(&serverAddress, 0, sizeof(serverAddress));

            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(SERVER_PORT);
            int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
            if (rval <= 0)
            {
                printf("inet_pton() failed");
                return -1;
            }

            // Make a connection to the server with socket SendingSocket.

            if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
            {
                printf("connect() failed with error code : %d", errno);
                continue;
            }

            printf("connected to server\n");

            // Sends some data to server

            FILE *fp;

            fp = fopen("./1mb.txt", "r");

            int data_stream;

            int total_send = 0;

            while ((data_stream = fread(buff, 1, sizeof(buff), fp)) > 0)
            {
                int bytesSent = send(sock, buff, data_stream, 0);
                total_send += bytesSent;
            }

            if (total_send == MBtoByte)
            {
                printf("sent all of the file\n");
            }
            else
            {
                printf("the data has been cut\n");
            }

            sleep(1); 
              
            close(sock);
            
        }
    }

    // TODO: All open clientSocket descriptors should be kept
    // in some container and closed as well.

    return 0;
}
