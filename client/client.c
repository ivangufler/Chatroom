#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "../colors.h"

#define PORT 23456

int main (int argc, char *argv[]) {

    if (argc < 2) {
        printc("Please enter an IP address: ./client [ip adress]\n", bold, red_f, 0);
        exit(0);
    }

    printf("Connecting... ");

    int clientsock = socket(AF_INET, SOCK_STREAM, 0);

    int value = 1;
    setsockopt(clientsock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &value, sizeof(value));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_family = AF_INET;

    value = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    if(value <= 0 || connect(clientsock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {

        printc("\nError: No connection to the server could be established.\n",
                bold, red_f, 0);
        exit(0);
    }

    printc("Success!\n", bold, green_f, 0);

    const int FLAGS = 0;

    char name[128] = { 0 };
    printc("Hello, what's your name? ", bold, default_f, 0);

    fgets(name, sizeof(name)-1, stdin);
    *strchr(name, '\n') = '\0';

    send(clientsock, name, strlen(name), FLAGS);

    printf("\n");
    int ret = 0;

    printf("\033[96m");
    fflush(stdout);

    int pid = fork();
    char exit[] = "/exit";

    if (pid == 0) {

        while (1) {
            char message[1024] = { 0 };

            printf("\033[96m");
            fflush(stdout);
            fgets(message, sizeof(message), stdin);
            *strchr(message, '\n') = '\0';

            if (strcmp(exit, message) == 0) {
                send(clientsock, exit, strlen(exit), FLAGS);
                break;
            }

            send(clientsock, message, strlen(message), FLAGS);
        }


    }

    else {
        while(1) {

            char buffer[1024] = {0};
            ret = recv(clientsock, buffer, sizeof(buffer), FLAGS);

            if (ret <= 0 || strcmp(exit, buffer) == 0) {
                break;
            }
            printf("\033[0m");
            fflush(stdout);
            printf("%s\n", buffer);
            printf("\033[96m");
            fflush(stdout);
        }

        if (ret <= 0) {
            printf("-> ");
            printc("Oops! Lost connection to the server.\n", 1, red_f, 0);
        }
        else {
            printf("-> ");
            printc("You left the chat. Bye.\n", 1, lightyellow_f, 0);
        }
    }

    close(clientsock);
    return 0;
}