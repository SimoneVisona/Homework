#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "draw.h"

void draw_position(double negativeWall, double positiveWall, double actual_position){
    //double length = positiveWall - negativeWall;
    draw_enviroment(negativeWall, positiveWall, actual_position);
    draw_values(negativeWall, positiveWall, actual_position);
    //printf("\r");
}

void draw_enviroment(double negativeWall, double positiveWall, double actual_position){
    printf(" ");
    for(int i=(negativeWall*10); i<((positiveWall*10)+1); i++){
        if (i == ((int)(actual_position*10)))
            printf("O");
        else if (i == (negativeWall*10) || i == (positiveWall*10))
            printf("|");
        else
            printf("-");
    }
    printf("\n \n");
}

void draw_values(double negativeWall, double positiveWall, double actual_position){
    for(int j=(negativeWall*10); j<((positiveWall*10)+1); j++){
        if (j == negativeWall*10)
            printf("%i", (int)j/10);
        else if (j == positiveWall*10)
            printf("%i", (int)j/10);
        else if (j == 0)
            printf("0");
        else if (j == ((int)(actual_position*10)))
            printf("%.1f", actual_position);
        else
            printf(" ");
    }
    printf("\n \n \n \n \n");
}
