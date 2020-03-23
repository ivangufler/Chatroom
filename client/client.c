#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include <stdio.h>
#include <sys/wait.h>

#define PORT 23456

int main (int argc, char *argv[]) {

    if (argc < 2) {
        printf("Please enter an IP address: ./client [ip adress]\n");
        exit(0);
    }

    int clientsock = socket(AF_INET, SOCK_STREAM, 0);

    int value = 1;
    setsockopt(clientsock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &value, sizeof(value));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_family = AF_INET;

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("Error: No connection to the server could be established.\n");
        exit(0);
    }

    connect(clientsock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    const int FLAGS = 0;

    char name[64] = { 0 };
    printf("Hello, what's your name? ");

    fgets(name, sizeof(name) - 1, stdin);
    *strchr(name, '\n') = '\0';

    send(clientsock, name, strlen(name), FLAGS);

    if (fork() == 0) {
        char buffer[1024] = { 0 };
        int ret = recv(clientsock, buffer, sizeof(buffer), FLAGS);
        printf("%s\n", buffer);
    }

   else {

       char exit[] = "exit";

       while(1) {

           char message[512] = { 0 };
           fgets(message, sizeof(message) - 1, stdin);
           *strchr(message, '\n') = '\0';

           if (strcmp(exit, message) == 0) {
               return 0;
           }

           send(clientsock, message, strlen(message), FLAGS);

       }
    }

}