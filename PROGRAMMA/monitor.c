#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "header_globalVariables.h"
#include "monitor.h"
#include "server.h"

pthread_mutex_t mtx;

pthread_cond_t deviceInput_notEmpty;
pthread_cond_t deviceInput_notFull;
pthread_cond_t devicePosition_notEmpty[2];
pthread_cond_t devicePosition_notFull;
pthread_cond_t serverIsReady;

void interface_startProducing()
{
    pthread_mutex_lock(&mtx);
        while(count == BUFFER_LENGTH){
            //printf("I'm the INTERFACE and I'll block my self \n");
            pthread_cond_wait(&deviceInput_notFull, &mtx);
        }
    pthread_mutex_unlock(&mtx);
}

void interface_endProducing()
{
    pthread_mutex_lock(&mtx);
        count++;
        //printf("Count : %i \n", count);
        pthread_cond_signal(&deviceInput_notEmpty);
    pthread_mutex_unlock(&mtx);
}

void model_startConsuming()
{
    pthread_mutex_lock(&mtx);
        while(count == 0){
            //printf("I'm the MODEL and I'll block my self because interface hasn't produce any date yet \n");
            pthread_cond_wait(&deviceInput_notEmpty, &mtx);
        }
    pthread_mutex_unlock(&mtx);
}

void model_endConsuming()
{
    pthread_mutex_lock(&mtx);
        count--;
        pthread_cond_signal(&deviceInput_notFull);
    pthread_mutex_unlock(&mtx);
}

void model_startProducing()
{
    pthread_mutex_lock(&mtx);
        while((count_ModelViewControl[VIEWER] == BUFFER_LENGTH) || (count_ModelViewControl[CONTROLLER] == BUFFER_LENGTH)){
            //printf("I'm the MODEL and I'll block my self because the viewer or the controller haven't read any position yet \n");
            pthread_cond_wait(&devicePosition_notFull, &mtx);
        }
    pthread_mutex_unlock(&mtx);
}

void model_endProducing()
{
    pthread_mutex_lock(&mtx);
        index_model = (index_model + 1)%BUFFER_LENGTH;
        count_ModelViewControl[VIEWER] ++;
        count_ModelViewControl[CONTROLLER]++;
        serverHasProduced = false;
        //printf("Count viewer: %i . Count controller %i\n", count_ModelViewControl[VIEWER], count_ModelViewControl[CONTROLLER]);
        pthread_cond_signal(&devicePosition_notEmpty[CONTROLLER]);
        pthread_cond_signal(&devicePosition_notEmpty[VIEWER]);
        pthread_cond_signal(&serverIsReady);
    pthread_mutex_unlock(&mtx);
}


void viewer_startView()
{
    pthread_mutex_lock(&mtx);
        while (count_ModelViewControl[VIEWER] == 0){
            //printf("I'm the VIEWER and I'll block my self \n");
            pthread_cond_wait(&devicePosition_notEmpty[VIEWER], &mtx);
        }
    pthread_mutex_unlock(&mtx);
}

void viewer_endView()
{
    pthread_mutex_lock(&mtx);
        count_ModelViewControl[VIEWER] = 0;
        //printf("Count viewer: %i \n", count_ModelViewControl[VIEWER]);
        if(count_ModelViewControl[VIEWER] >= count_ModelViewControl[CONTROLLER]){
            //printf("I'm the VIEWER and I'm signaling the model \n");
            pthread_cond_signal(&devicePosition_notFull);
        }

    pthread_mutex_unlock(&mtx);
}

void controller_startControl()
{
    pthread_mutex_lock(&mtx);
        while (count_ModelViewControl[CONTROLLER] == 0){
            //printf("I'm the CONTROLLER and I'll block my self \n");
            pthread_cond_wait(&devicePosition_notEmpty[CONTROLLER], &mtx);
        }
    pthread_mutex_unlock(&mtx);
}

void controller_endControl()
{
    pthread_mutex_lock(&mtx);
        count_ModelViewControl[CONTROLLER]--;
        //printf("Count controller: %i \n", count_ModelViewControl[CONTROLLER]);
        if (count_ModelViewControl[CONTROLLER] >= count_ModelViewControl[VIEWER]){
            //printf("I'm the CONTROLLER and I'm signaling the model \n");
            pthread_cond_signal(&devicePosition_notFull);
        }
    pthread_mutex_unlock(&mtx);
}

void Kill(int *socket, int *server_status, char disconnection_msg[]){
    pthread_mutex_lock(&mtx);
        //HO CERCATO DI CREARE IL PROGRAMMA IN MODO TALE CHE OGNI THREAD A FINE VITA SVEGLIASSI 'QUELLA SOPRA'; VIEWER E CONTROLLER SVEGLIANO MODEL
        //AMENTRE MODEL SVEGLIA INTERFACE. A QUESTO PUNTO NON OCCORRE SVEGLIARE NESSUNO. IL PROGRAMMA SEGUE IL SUO
        //FUNZIONAMENTO NORMALE PER UN SOLO CICLO, POI OGNI THREAD E' PORTATA A MORIRE
        /*pthread_cond_signal(&deviceInput_notEmpty);
        pthread_cond_signal(&deviceInput_notFull);
        pthread_cond_signal(&devicePosition_notEmpty[0]);
        pthread_cond_signal(&devicePosition_notEmpty[1]);
        pthread_cond_signal(&devicePosition_notFull);*/
        done = true;

        if((*server_status) == CONNECTION_IS_ONLINE){
            if (send((*socket), disconnection_msg, MSG_OUT_LENGTH, 0) == -1)
                perror("send");
        }

        sleep(2);

        closeConnection();

    pthread_mutex_unlock(&mtx);
}

void signalC_Viewermodel(){
    pthread_mutex_lock(&mtx);
    count_ModelViewControl[VIEWER] = 1;
    //count_ModelViewControl[CONTROLLER]--;
    pthread_cond_signal(&devicePosition_notFull);
    pthread_mutex_unlock(&mtx);
}

void signalC_Controllermodel(){
    pthread_mutex_lock(&mtx);
    count_ModelViewControl[CONTROLLER] = 1;
    //count_ModelViewControl[CONTROLLER]--;
    pthread_cond_signal(&devicePosition_notFull);
    pthread_mutex_unlock(&mtx);
}

void signalC_interface(){
    pthread_mutex_lock(&mtx);
    count = 1;
    pthread_cond_signal(&deviceInput_notFull);
    pthread_mutex_unlock(&mtx);
}

void server_startProducing(int socket){
    pthread_mutex_lock(&mtx);
    while(serverHasProduced){
        /*if (send(socket, "Please wait before sending another position \n", MSG_OUT_LENGTH, 0) == -1)
            perror("send");
        printf("Server bloccato \n");*/
        pthread_cond_wait(&serverIsReady, &mtx);
        /*printf("Server sbloccato \n");
        if (send(socket, "The server is ready again. Put the position: \n", MSG_OUT_LENGTH, 0) == -1)
            perror("send");*/
    }
    pthread_mutex_unlock(&mtx);
}

void server_Produce(double value){
    pthread_mutex_lock(&mtx);
    serverPosition = value;
    serverHasProduced = true;
    pthread_mutex_unlock(&mtx);
}

double updatePosition(double buffer[], struct Input *input_from_prompt, double actualValue, int N){
    double value;
    value = buffer[N] + actualValue;

    if (value < (*input_from_prompt).positionW1){
        value = (*input_from_prompt).positionW1;
        printf("\a");
    }

    if(value > (*input_from_prompt).positionW2){
        value = (*input_from_prompt).positionW2;
        printf("\a");
    }

    //printf("I'm the MODEL and I have consumed the date number %i. At this time the device's position is: %f \n", N, value);
    return value;
}


void destroy_mutex_condvar(){
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&deviceInput_notEmpty);
    pthread_cond_destroy(&deviceInput_notFull);
    pthread_cond_destroy(&devicePosition_notEmpty[0]);
    pthread_cond_destroy(&devicePosition_notEmpty[1]);
    pthread_cond_destroy(&devicePosition_notFull);
    pthread_cond_destroy(&serverIsReady);
}
