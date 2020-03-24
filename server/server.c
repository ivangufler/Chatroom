#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <time.h>
#include "server.h"

#define PORT 23456

const int FLAGS = 0;
const int MAX_CONNECTIONS = 200;

int main(void) {

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    int value = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &value, sizeof(value));

    //non-blocking connections
    ioctl(sock, FIONBIO, (char *)&value);

    //bind to port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(sock, (const struct sockaddr *) &addr, sizeof(addr));

    //start tcp listening socket
    int backlog = 5;
    listen(sock, backlog);

    char *names[128] = { 0 };

    //poll components
    struct pollfd fds[MAX_CONNECTIONS];
    int nfds = 2, currsize = 0;

    memset(fds, 0 , sizeof(fds));

    fds[0].fd = 0;
    fds[0].events = POLLIN;

    fds[1].fd = sock;
    fds[1].events = POLLIN;

    char exit[] = "/exit";
    int client = 0;
    int ret = 0;

    int stop = 0;
    int compress = 0;

    do {

        compress = 0;

        printf("Waiting on poll...\n");
        ret = poll(fds, nfds, -1);

        if (ret < 0) {
            perror("poll failed");
            break;
        }

        currsize = nfds;
        for (int i = 0; i < currsize; i++) {

            if (fds[i].revents == 0) {
                continue;
            }

            if (fds[i].revents != POLLIN) {
                printf("   Error: descriptor %i, revents %i\n", fds[i].fd, fds[i].revents);
                break;
            }

            if (fds[i].fd == 0) {

                char buffer[64] = { 0 };
                read(0, buffer, sizeof(buffer));
                *strchr(buffer, '\n') = '\0';

                if (strcmp(buffer, "stop") == 0) {
                    stop = 1;
                    break;
                }
                continue;
            }

            if (fds[i].fd == sock) {

                printf("   Ready for incoming connections\n");

                do {

                    struct sockaddr clientaddr;
                    socklen_t clientaddrlen;
                    client = accept(sock, &clientaddr, &clientaddrlen);

                    if (client < 0) {
                        if (errno != EWOULDBLOCK) {
                            //huge problem
                            stop = 1;
                        }
                        break;
                    }

                    if (nfds == MAX_CONNECTIONS) {
                        printf("   Maximum number of connections reached!");
                        send(client, "\033[1;31mSorry, the chat room is full. Try again later.\033[0m",
                                57, FLAGS);
                        close(client);
                        break;
                    }
                    printf("   New connection - %i\n", client);
                    fds[nfds].fd = client;
                    fds[nfds].events = POLLIN;
                    nfds++;

                } while (client != -1);
            }

            else {

                int closeconn = 0;
                printf("   Ready for reading from descriptor %i\n", fds[i].fd);

                //set recv function non blocking
                fcntl(fds[i].fd, F_SETFL, fcntl(fds[i].fd, F_GETFL) | O_NONBLOCK);

                do {
                    char buffer[1024] = { 0 };
                    ret = recv(fds[i].fd, buffer, sizeof(buffer), FLAGS);
                    char *tmp = calloc(strlen(buffer) + 150, sizeof(char));

                    if (ret < 0) {
                        if (errno != EWOULDBLOCK) {
                            //huge problem
                            stop = 1;
                            closeconn = 1;
                        }
                        break;
                    }

                    if (ret == 0) {
                        printf("   Connection to %i closed\n", fds[i].fd);
                        strcpy(tmp, "-> ");
                        strcat(tmp, names[i-2]);
                        strcat(tmp, "left the chat\0");
                        *strchr(tmp, ':') = ' ';
                        broadcast(fds, fds[i], tmp, currsize);

                        closeconn = 1;
                        break;
                    }

                    printf("   received: %s\n", buffer);

                    if (strcmp(exit, buffer) == 0) {
                        send(fds[i].fd, buffer, strlen(buffer), FLAGS);
                        break;
                    }

                    if (names[i - 2] == NULL) {
                        names[i-2] = calloc(strlen(buffer) + 12, sizeof(char));

                        char buf[9] = { 0 };
                        int c = color();
                        snprintf(buf, 9, "\033[1;%im", c);

                        strcpy(names[i-2], buf);
                        strcat(names[i - 2], buffer);
                        strcat(names[i - 2], ":\033[0m");

                        strcpy(tmp, "-> ");
                        strcat(tmp, names[i-2]);
                        *strchr(tmp, ':') = ' ';
                        strcat(tmp, "is now online\0");
                    }
                    else {
                        strcpy(tmp, names[i - 2]);
                        strcat(tmp, " ");
                        strcat(tmp, buffer);
                        strcat(tmp, "\0");
                    }
                    broadcast(fds, fds[i], tmp, currsize);

                } while (1);

                if (closeconn) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    names[i - 2] = NULL;
                    compress = 1;
                }

            }

        }

        if (compress) {
            for (int i = 0; i < nfds; i++) {

                if (fds[i].fd == -1) {
                    for(int j = i; j < nfds; j++) {
                        fds[j].fd = fds[j+1].fd;

                        if (names[j-1] == NULL) {
                            names[j-2] = NULL;
                        }
                        else {
                            names[j-2] = calloc(strlen(names[j-1]), sizeof(char));
                            strcpy(names[j-2], names[j-1]);
                        }
                    }
                    i--;
                    nfds--;
                }
            }
        }


    } while(stop == 0);

    printf("Stopping the server... ");

    for (int i = 2; i < nfds; i++) {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }

    close(fds[1].fd);
    printf("Bye.\n");
    return 0;
}

int color() {

    // 32, 33, 34, 35, 92, 93, 94, 95
    static int colors[8];
    for (int i = 0; i < 4; i++) {
        colors[i] = 32 + i;
        colors[i + 4] = 92 + i;
    }
    time_t t;
    srand(time(&t) + 5);
    int i = rand() % 10;

    return colors[i];

}

void broadcast(struct pollfd *fds, struct pollfd actual, char *msg, int currsize) {

    for (int j = 2; j < currsize; j++) {

        if (fds[j].fd != actual.fd) {
            int ret = send(fds[j].fd, msg, strlen(msg), FLAGS);
            printf("   r an %i: %i\n", fds[j].fd, ret);
        }
    }
}
