#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

#include "header_globalVariables.h"
#include "parsingInput.h"
#include "threads.h"
#include "monitor.h"
#include "draw.h"

/*                                               G L O B A L       V A R I A B L E S                                                 */

double DeviceInput[BUFFER_LENGTH];
double DevicePosition[BUFFER_LENGTH];

int count = 0;
int count_ModelViewControl[2] = {0,0};

int index_model = 0;
double serverPosition;

bool serverHasProduced = false;
bool done = false;

int new_fd;
int server_online;


int main(int argc, char *argv[]){

int s;
struct Input inputs;

sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);

s = pthread_sigmask(SIG_BLOCK, &set, NULL);
if (s != 0)
    printf("errore \n");

if (!checkInput(argc, argv))
    exit(EXIT_FAILURE);

parseInput(argc, argv, &inputs);

viewData(&inputs);

double clockTimeInterface = detect_CycleTime(); //parsingInput's function that use file "weightedAverage.c"

printf("The moving sensor's cycle time is estimated being: %f \n", clockTimeInterface);

sleep(3);

/*system("clear");

sleep(5);*/

pthread_t interface;
pthread_t model;
pthread_t viewer;
pthread_t controller;
pthread_t signalListener;
pthread_t server;

s = pthread_create(&interface, NULL, (void*)interface_Function, &clockTimeInterface);
if (s != 0)
    printf("Error thread interface \n");

s = pthread_create(&model, NULL, (void*)model_Function, &inputs);
if (s != 0)
    printf("Error thread model \n");

s = pthread_create(&viewer, NULL, (void*)viewer_Function, &inputs);
if (s != 0)
    printf("Error thread viewer \n");

s = pthread_create(&controller, NULL, (void*)controller_Function, &inputs);
if (s != 0)
    printf("Error thread controller \n");

s = pthread_create(&signalListener, NULL, (void*)listener_Function, &set);
if (s != 0)
    printf("Error thread signal listener \n");

s = pthread_create(&server, NULL, (void*)server_Function, &inputs);
if (s != 0)
    printf("Error thread server \n");

s =  pthread_join(interface, NULL);
if (s != 0)
    printf("Error join thread interface \n");

s =  pthread_join(model, NULL);
if (s != 0)
    printf("Error join thread model \n");

s =  pthread_join(viewer, NULL);
if (s != 0)
    printf("Error join thread model \n");

s =  pthread_join(controller, NULL);
if (s != 0)
    printf("Error join thread model \n");

s =  pthread_join(signalListener, NULL);
if (s != 0)
    printf("Error join thread signal listener \n");

s =  pthread_join(server, NULL);
if (s != 0)
    printf("Error join thread server \n");

destroy_mutex_condvar();

exit(EXIT_SUCCESS);
}
