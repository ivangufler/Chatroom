#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <stdio.h>

#define PORT 23456

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

    //poll components
    struct pollfd fds[200];
    int nfds = 1, currsize = 0;

    memset(fds, 0 , sizeof(fds));
    fds[0].fd = sock;
    fds[0].events = POLLIN;

    const int FLAGS = 0;
    int client = 0;
    int ret = 0;

    do {

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

            if (fds[i].fd == sock) {

                printf("   Ready for incoming connections\n");

                do {

                    struct sockaddr clientaddr;
                    socklen_t clientaddrlen;
                    client = accept(sock, &clientaddr, &clientaddrlen);

                    if (client < 0) {
                        if (errno != EWOULDBLOCK) {
                            //huge problem
                        }
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

                    if (ret < 0) {
                        if (errno != EWOULDBLOCK) {
                            //huge problem
                            closeconn = 1;
                        }
                        break;
                    }

                    if (ret == 0) {
                        printf("   Connection to %i closed\n", fds[i].fd);
                        closeconn = 1;
                        break;
                    }

                    //received a message
                    //send it to all other users

                    printf("   received: %s\n", buffer);
                    for (int j = 1; j < currsize; j++) {
                        if (fds[j].fd != fds[i].fd) {
                            ret = send(fds[j].fd, buffer, strlen(buffer), FLAGS);
                            printf("   r an %i: %i\n", fds[i].fd, ret);
                        }
                    }

                } while (1);

                if (closeconn) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }

            }

        }

    } while(1);

    return 0;
}