#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <poll.h>

#include "../colors/colors.h"

#define PORT 23456

int main (int argc, char *argv[]) {

    //ip adress as argument
    if (argc < 2) {
        printc("Please enter an IP address: ./client [ip adress]\n", bold, red_f, 0);
        exit(0);
    }

    printf("Connecting... ");

    //create socket
    int clientsock = socket(AF_INET, SOCK_STREAM, 0);

    int value = 1;
    setsockopt(clientsock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &value, sizeof(value));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_family = AF_INET;

    value = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    //establish a connection
    if(value <= 0 ||
    connect(clientsock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {

        printc("\nError: No connection to the server could be established.\n",
                bold, red_f, 0);
        exit(0);
    }

    printc("Success!\n", bold, green_f, 0);

    const int FLAGS = 0;

    //Save the name of the user
    char name[128] = { 0 };
    printc("Hello, what's your name? ", bold, default_f, 0);

    fgets(name, sizeof(name)-1, stdin);
    *strchr(name, '\n') = '\0';
    //send the name to the server
    send(clientsock, name, strlen(name), FLAGS);

    printf("\n\033[96m");
    fflush(stdout);

    const int nfds = 2;
    struct pollfd fds[nfds];
    memset(fds, 0 , sizeof(fds));

    //save stdin in fd-array
    fds[0].fd = 0;
    fds[0].events = POLLIN;

    //save the socket in fd-array
    fds[1].fd = clientsock;
    fds[1].events = POLLIN;

    int ret = 0;
    int stop = 0;

    do {
        //poll, no timeout
        ret = poll(fds, nfds, -1);

        //error on polling
        if (ret < 0) {
            perror("# ERROR: poll failed");
            break;
        }
        //for stdin and the socket
        for (int i = 0; i < nfds; i++) {

            //nothing happened, ignore
            if (fds[i].revents == 0) {
                continue;
            }

            char buffer[1024] = { 0 };

            //there is data on stdin
            if (fds[i].fd == 0) {
                //read the data from the terminal
                read(0, buffer, sizeof(buffer));
                *strchr(buffer, '\n') = '\0';

                //clear terminal if command was read
                if (strcmp("/clear", buffer) == 0) {
                    system("clear");
                }
                else {
                    //send the input data to the server
                    send(clientsock, buffer, strlen(buffer), FLAGS);
                }

            }
            //there is data on the socket
            else {
                //receive the data from the server
                ret = recv(clientsock, buffer, sizeof(buffer), FLAGS);

                //if the data is the exit command, stop the client
                if (ret <= 0 || strcmp("/exit", buffer) == 0) {
                    stop = 1;
                    break;
                }
                printf("\033[0m");
                fflush(stdout);
                printf("%s\n", buffer);
                printf("\033[96m");
                fflush(stdout);
            }
        }
    //until stop is setted to 1
    } while(stop == 0);

    //print an arrow
    printf("\033[0m");
    fflush(stdout);
    printf("-> ");

    //print a final message
    if (ret <= 0) {
        printc("Oops! Lost connection to the server.\n", 1, red_f, 0);
    }
    else {
        printc("You left the chat. Bye.\n", 1, lightyellow_f, 0);
    }
    //close the client socket
    close(clientsock);
    return 0;
}
