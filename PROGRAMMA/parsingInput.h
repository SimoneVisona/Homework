#ifndef parsingInput_h
#define parsingInput_h

void printHelp();

int checkInput(int numArg, char **arguments);

void parseInput(int numArg, char **arguments, struct Input *input_fromTerminal);

void viewData(struct Input *input_fromTerminal);

double detect_CycleTime();

#endif
