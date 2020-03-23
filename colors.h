#ifndef CHATROOM_COLORS_H
#define CHATROOM_COLORS_H

    static const int normal_i = 0;
    static const int white_i = 30;
    static const int red_i = 31;
    static const int green_i = 32;
    static const int yellow_i = 33;
    static const int blue_i = 34;
    static const int magenta_i = 35;
    static const int cyan_i = 36;

    static const char *normal_c = "\033[0m";
    static const char *white_c = "\033[0;30m";
    static const char *red_c = "\033[0;31m";
    static const char *green_c = "\033[0;32m";
    static const char *yellow_c = "\033[0;33m";
    static const char *blue_c = "\033[0;34m";
    static const char *magenta_c = "\033[0;35m";
    static const char *cyan_c = "\033[0;36m";

    void red(char *text);
    void green(char *text);
    void yellow(char *text);
    void blue(char *text);
    void magenta(char *text);
    void cyan(char *text);
    void custom(int bold, int color, char *text);

#endif //CHATROOM_COLORS_H
