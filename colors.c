#include <stdio.h>
#include "colors.h"

void color(char *text, int color, int background, int set) {

    if (background == 0)
        background = 49;

    printf("\033[%i;%i;%im%s\033[0m", set, background, color, text);

}