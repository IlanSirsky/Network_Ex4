/*
    TCP/IP-server
*/

#include <stdio.h>

// Linux and other UNIXes
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define SERVER_PORT 5060 //The port that the server listens

int main()
{
    char buffer[1024];
    // Open the listening (server) socket
    int listeningSocket = -1;

    if ((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Could not create listening socket : %d", errno);
    }

    // Reuse the address if the server socket on was closed
    // and remains for 45 seconds in TIME-WAIT state till the final removal.
    //
    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0)
    {
        printf("setsockopt() failed with error code : %d", errno);
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT); //network order

    // Bind the socket to the port with any IP at this port
    if (bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        printf("Bind failed with error code : %d", errno);
        // TODO: close the socket
        return -1;
    }

    printf("Bind() success\n");

    // Make the socket listening; actually mother of all client sockets.
    if (listen(listeningSocket, 500) == -1) //500 is a Maximum size of queue connection requests
        //number of concurrent connections
    {
        printf("listen() failed with error code : %d", errno);
        // TODO: close the socket
        return -1;
    }

    //Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");

    struct sockaddr_in clientAddress; //
    socklen_t clientAddressLen = sizeof(clientAddress);

    for (int i = 0; i < 2; i++)
    {

        if (listeningSocket == -1)
        {
            printf("Could not create socket : %d", errno);
        }

        socklen_t len = sizeof(buffer);
        if (getsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, &len) != 0)
        {
            perror("getsockopt");
            return -1;
        }

        strcpy(buffer, i > 0 ? "reno" : "cubic");

        len = strlen(buffer);
        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, len) != 0)
        {
            perror("setsockopt");
            return -1;
        }
        len = sizeof(buffer);
        if (getsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, &len) != 0)
        {
            perror("getsockopt");
            return -1;
        }

        double avg = 0.0;
        for (int j = 0; j < 5; j++)
        {
            memset(&clientAddress, 0, sizeof(clientAddress));
            clientAddressLen = sizeof(clientAddress);
            int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
            if (clientSocket == -1)
            {
                printf("listen failed with error code : %d", errno);
                // TODO: close the sockets
                return -1;
            }

            printf("Starting to measure time!\n");

            struct timespec start, end;
            clock_gettime(CLOCK_REALTIME, &start);

            int bytes = -1;
            while (bytes != 0)
            {
                bytes = recv(clientSocket, buffer, 1024, 0);
            }
            clock_gettime(CLOCK_REALTIME, &end);
            // time_spent = end - start
            double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
            printf("Time for package %d is %f seconds\n\n", i+j, time_spent);

            avg += time_spent;

            sleep(1);
        }
        printf("\nAVG time for %s is %f\n\n", i > 0 ? "reno" : "cubic", avg / 5);
    }

    // TODO: All open clientSocket descriptors should be kept
    // in some container and closed as well.
    close(listeningSocket);

    return 0;
}
