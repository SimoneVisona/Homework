#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include "header_globalVariables.h"
#include "parsingInput.h"
#include "weightedAverage.h"

void printHelp() {
  printf("Usage: ./main Position_first_wall (<0) | Position_secondWall (>0) | Tcycle Viewer (>0) | Tcycle Controller (>0) \n ");
}

int checkInput(int numArg, char **arguments) {
  if (numArg != 5 ) { //controllo che da terminale passo 4 argomenti (posizione mura; tempi viewer - controller)
    printHelp();
    return true;
  }

  if (atoi(arguments[1]) > 0){ //controllo che il secondo muro sia nella parte a sinistra del centro (la posizione iniziale è 0)
    printHelp();
    return true;
  }

  for (int i=2; i<numArg; i++){ //controllo che i parametri del tempo e del secondo muro siano positivi
    if (atof(arguments[i]) < 0){
        printf("Error: the %i element have not to be negative. Please follow next rules: \n", i);
        printHelp();
        return false;
    }
  }

  for (int i=2; i<numArg; i++){
    if (atof(arguments[i]) == 0){
        printf("Error: the %i element have not to be zero. Please follow next rules: \n", i);
        printHelp();
        return false;
    }
  }

  return true;
}

void parseInput(int numArg, char **arguments, struct Input *input_fromTerminal) {
    (*input_fromTerminal).positionW1 = atof(arguments[1]);
    (*input_fromTerminal).positionW2 = atof(arguments[2]);
    (*input_fromTerminal).clockTimeViewer = atof(arguments[3]);
    (*input_fromTerminal).clockTimeController = atof(arguments[4]);
}

void viewData(struct Input *input_fromTerminal){
    printf("The inputs that have been demanded are: \n First wall position: %f \n Second wall position: %f \n Viewer's time cycle: %f \n Controller's time cycle: %f \n \n \n",
           (*input_fromTerminal).positionW1, (*input_fromTerminal).positionW2, (*input_fromTerminal).clockTimeViewer, (*input_fromTerminal).clockTimeController);
}

double detect_CycleTime(){
    FILE *tuning_file;
    char line[MAX_LINE_LENGTH];
    int status = 1;

    tuning_file = fopen("tuning_device.txt", "r");

    if (tuning_file == NULL) {
        printf("Can't open tuning_device.txt\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    double time_array[MAX_TUNING_VALUES];
    double time;
    double res;

    while ( fgets(line, sizeof(line), tuning_file)) {

        sscanf(line, "%le", &time);
        time_array[index] = time;
        index ++;
        if (index < MAX_TUNING_VALUES){
        } else {
            fclose(tuning_file);
            status = 0;
            printf("The tuning file hasn't being read at all, there are too much values. \n ");
            printf("I'll find the weighted average based on the values that I managed to save \n");
        }
    }
    fclose(tuning_file);
    if (status > 0)
        printf("The tuning file has being read at all \n");

    //Now I have all time input and I can look for them weighted average
    //res = weighted_average(time_array, MAX_TUNING_VALUES);
    res = average(time_array, MAX_TUNING_VALUES);
    return res;
}

