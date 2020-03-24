#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

    void broadcast(struct pollfd *fds, struct pollfd actual, char *msg, int currsize, char **names);
    int color();
    int command(char *command);

#endif //CHATROOM_SERVER_H
