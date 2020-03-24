#ifndef CHATROOM_COLORS_H
#define CHATROOM_COLORS_H

    // Foreground (text) colors
    static const int default_f = 39;
    static const int black_f = 30;
    static const int red_f = 31;
    static const int green_f = 32;
    static const int yellow_f = 33;
    static const int blue_f = 34;
    static const int magenta_f = 35;
    static const int cyan_f = 36;
    static const int lightgray_f = 37;
    static const int darkgray_f = 90;
    static const int lightred_f = 91;
    static const int lightgreen_f = 92;
    static const int lightyellow_f = 93;
    static const int lightblue_f = 94;
    static const int lightmagenta_f = 95;
    static const int lightcyan_f = 96;
    static const int white_f = 97;

    // Background colors
    static const int default_b = 49;
    static const int black_b = 40;
    static const int red_b = 41;
    static const int green_b = 42;
    static const int yellow_b = 43;
    static const int blue_b = 44;
    static const int magenta_b = 45;
    static const int cyan_b = 46;
    static const int lightgray_b = 47;
    static const int darkgray_b = 100;
    static const int lightred_b = 101;
    static const int lightgreen_b = 102;
    static const int lightyellow_b = 103;
    static const int lightblue_b = 104;
    static const int lightmagenta_b = 105;
    static const int lightcyan_b = 106;
    static const int white_b = 107;

    // text sets
    static const int bold = 1;
    static const int dim = 2;
    static const int underlined = 4;
    static const int blink = 5;
    static const int reverse = 7;
    static const int hidden = 8;

    void printc(char *text, int set, int color, int background);

    #endif //CHATROOM_COLORS_H
