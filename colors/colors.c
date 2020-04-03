#include <stdio.h>
#include "colors.h"

void printc(char *text, int set, int color, int background) {

    if (background == 0)
        background = 49;

    printf("\033[%i;%i;%im%s\033[0m", set, background, color, text);

}