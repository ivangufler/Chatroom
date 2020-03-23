#include <stdio.h>
#include "colors.h"


void bold(char *text) {
    printf("%s%s%s", bold_c, text, normal_c);
}

void red(char *text) {
    printf("%s%s%s", red_c, text, normal_c);
}

void green(char *text) {
    printf("%s%s%s", green_c, text, normal_c);
}

void yellow(char *text) {
    printf("%s%s%s", yellow_c, text, normal_c);
}

void blue(char *text) {
    printf("%s%s%s", blue_c, text, normal_c);
}

void magenta(char *text) {
    printf("%s%s%s", magenta_c, text, normal_c);
}

void cyan(char *text) {
    printf("%s%s%s\n", cyan_c, text, normal_c);
}

void custom(int bold, int color, char *text) {
    printf("\033[%i;%im%s\033[0m", bold, color, text);
}