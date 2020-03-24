#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "server.h"
#include "../colors.h"

#define PORT 23456

const int FLAGS = 0;
const int MAX_CONNECTIONS = 200;

time_t begin;


int main(void) {

    printf("Starting the server... ");

    begin = time(NULL);

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

    char *names[200] = { 0 };
    int total_logins = 0;

    //poll components
    struct pollfd fds[MAX_CONNECTIONS];
    int nfds = 2, currsize = 0, curronline = 0;

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

    printc("Success!\n", bold, green_f, 0);
    printf("# INFO: Type \"help\" if you want to know more\n\n");

    do {

        compress = 0;

        ret = poll(fds, nfds, -1);

        if (ret < 0) {
            perror("# ERROR: poll failed");
            break;
        }

        currsize = nfds;
        for (int i = 0; i < currsize; i++) {

            if (fds[i].revents == 0) {
                continue;
            }

            if (fds[i].revents != POLLIN) {
                printf("# ERROR: descriptor %i, revents %i\n", fds[i].fd, fds[i].revents);
                fds[i].fd = -1;
                compress = 1;
                break;
            }

            if (fds[i].fd == 0) {

                char buffer[64] = { 0 };
                read(0, buffer, sizeof(buffer));

                ret = command(buffer);

                if (ret == -1) {
                    printf("\tThis command doesn't exist.\n");
                    printf("\tType \"help\" for a list of all available commands.\n");
                }

                else if (ret == 1) {
                    stop = 1;
                    break;
                }

                else if(ret == 2) {

                    //Stats
                    char hostbuffer[256];
                    char *IPbuffer;
                    struct hostent *host_entry;
                    int hostname;

                    // To retrieve hostname
                    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
                    host_entry = gethostbyname(hostbuffer);

                    IPbuffer = inet_ntoa(*((struct in_addr *)
                            host_entry->h_addr_list[0]));

                    printf("\tIt is running at %s (%s) on port %i\n",
                           IPbuffer, hostbuffer, PORT);

                    printf("\tThere have been %i logins since the server was started.\n",
                           total_logins);
                }
                else if (ret > 2) {
                    //Users
                    printf("\tActually, %i users are online:\n", curronline);

                    if (curronline == 0) {
                        printf("\tThere is no user on the server.\n");
                    }

                    for (int m = 0; m < curronline; m++) {

                        char name[200] = { 0 };
                        strcpy(name, names[m]);
                        *strchr(name, ':') = '\n';
                        printf("\t\t%i: %s", (m+1), name);
                    }

                    if (ret == 4 && curronline > 0) {

                        int kick = 0;
                        char c[8] = { 0 };

                        write(1, "\033[0m\tWhich user should be kicked? ", 34);
                        read(0, c, sizeof(c));
                        *strchr(c, '\n') = '\0';

                        kick = atoi(c);

                        if (kick <= 0 || kick > curronline) {
                            write(1, "ID not valid.\n", 14);
                        }
                        else {

                            char name[200] = {0};
                            strcpy(name, names[kick - 1]);
                            *strchr(name, ':') = ' ';

                            write(1, "\t", 1);
                            write(1, name, strlen(name));
                            write(1, "(", 1);
                            write(1, c, strlen(c));

                            send(fds[kick+1].fd, "/exit",
                                 5, FLAGS);

                            compress = 1;
                            close(fds[kick+1].fd);
                            curronline--;
                            write(1, ") was kicked off the server.\n", 29);
                        }

                    }
                }
                continue;
            }

            if (fds[i].fd == sock) {

                printf("# INFO: Ready for incoming connections\n");

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
                        printf("# WARNING: Maximum number of connections reached!\n");
                        send(client, "\033[1;31mSorry, the chat room is full. Try again later.\033[0m",
                                57, FLAGS);
                        close(client);
                        break;
                    }
                    printf("# INFO: New connection, id=%i\n", client-3);
                    fds[nfds].fd = client;
                    fds[nfds].events = POLLIN;
                    nfds++;

                } while (client != -1);
            }

            else {

                int closeconn = 0;

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
                        printf("# INFO: Connection closed, id=%i\n", fds[i].fd-3);

                        if (names[i - 2] != NULL) {

                            strcpy(tmp, "-> ");
                            strcat(tmp, names[i - 2]);
                            strcat(tmp, "left the chat\0");
                            *strchr(tmp, ':') = ' ';
                            broadcast(fds, fds[i], tmp, currsize);
                            curronline--;
                        }
                        closeconn = 1;
                        break;
                    }

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

                        total_logins++;
                        curronline++;
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
    return colors[(rand() % 8)];

}

void broadcast(struct pollfd *fds, struct pollfd actual, char *msg, int currsize) {

    for (int j = 2; j < currsize; j++) {

        if (fds[j].fd != actual.fd) {
            int ret = send(fds[j].fd, msg, strlen(msg), FLAGS);
        }
    }
}

int command(char *command) {

    int ret = -1;
    *strchr(command, '\n') = '\0';

    if (strcmp(command, "stop") == 0) {
        ret = 1;
    }
    else if (strcmp(command, "stats") == 0) {

        time_t end = time(NULL);

        long hours = 0;
        long minutes = 0;
        long seconds = (end - begin);

        minutes = seconds / 60;
        seconds = seconds % 60;

        hours = minutes / 60;
        minutes = minutes % 60;
        printf("\tThe server is online since %li hours, %li minutes, %li seconds\n",
                hours, minutes, seconds);

        ret = 2;

    }
    else if (strcmp(command, "users") == 0) {
        ret = 3;
    }

    else if (strcmp(command, "kick") == 0) {
        ret = 4;
    }
    else if (strcmp(command, "help") == 0) {
        printf("You can use the following commands:\n");
        printf("   stop \t//stopping the server\n");
        printf("   stats\t//showing some informations about the server\n");
        printf("   users\t//showing the online users\n");
        printf("   kick \t//removing users from the server\n");
        printf("   help \t//showing this help page\n\n");
        ret = 0;
    }
    return ret;
}