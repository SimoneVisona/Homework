#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

/*                                   S O C K E T      R E Q U I R E D       L I B R A R I E S                                   */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "header_globalVariables.h"
#include "threads.h"
#include "monitor.h"
#include "draw.h"
#include "server.h"


/*                                               T H R E A D S      F U N C T I O N S                                            */


/* THIS IS THE GLOBAL SOCKET THAT I CAN USE IN "KILL FUNCTION" */

void* interface_Function(double* clockTimeInterface)
{
    FILE *device_file;
    char line[MAX_LINE_LENGTH];
    int status = 0;
    device_file = fopen("device.txt", "r");

    if (device_file == NULL) {
        printf("Can't open device.txt\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    int time;
    double change;

    while ( fgets(line, sizeof(line), device_file)) {
        sleep(CYCLE_TIME*(*clockTimeInterface));
        sscanf(line, "%d %lf", &time, &change);
        interface_startProducing(); /*OPERATION*/

                DeviceInput[index] = change;

                //printf("I'm the INTERFACE and I have produced the data number %i. The detected position is: %f \n", index, change);

        interface_endProducing(); /*OPERATION*/

        index = (index + 1)%BUFFER_LENGTH;

        if(done == true){
            status = 1;
            printf("I'm the interface thread and I'll die \n");
            pthread_exit(NULL);
        }
    }
    fclose(device_file);
    if (status < 1)
        printf("The file has being read at all \n");

    printf("I'm the interface thread and I'll die \n");
    pthread_exit(NULL);
}

void* model_Function(struct Input *inputs) //necessario passare la struttura dati al fine di non generare un'altra variabile globale
{
    struct Input *input_from_prompt;
    input_from_prompt = inputs;
    //int index = 0;
    double NewPosition = 0;
    while(!done){

        model_startConsuming(); /*OPERATION*/

            NewPosition = updatePosition(DeviceInput, input_from_prompt, NewPosition, index_model);

        model_startProducing(); /*OPERATION*/

            if (serverHasProduced && (serverPosition != DISCONNECT_NUMBER)){
                NewPosition = serverPosition;
                DevicePosition[index_model] = NewPosition;
                //printf("I'm the MODEL and CLIENT required me to se the position at: %f \n", DevicePosition[index_model]);
            } else {
                DevicePosition[index_model] = NewPosition;
                //printf("I'm the MODEL and the position at the index %i is %f \n", index_model, DevicePosition[index_model]);
            }

        model_endProducing();  /*OPERATION*/

        model_endConsuming(); /*OPERATION*/

    }
    signalC_interface();
    printf("I'm the model thread and I'll die \n");
    pthread_exit(NULL);
}

void* viewer_Function(struct Input *inputs)
{
    double value;
    while(!done) {
        viewer_startView(); /*OPERATION*/

            if (index_model > 0){
                value = DevicePosition[index_model - 1];
                printf("I'm the VIEWER: the most recent position is: %f \n", DevicePosition[index_model - 1]);
                //draw_position((*inputs).positionW1, (*inputs).positionW2, DevicePosition[index_model - 1]);
            }
            else{
                value = DevicePosition[BUFFER_LENGTH - 1];
                //draw_position((*inputs).positionW1, (*inputs).positionW2, DevicePosition[BUFFER_LENGTH - 1]);
                printf("I'm the VIEWER: the most recent position is: %f \n", DevicePosition[BUFFER_LENGTH - 1]);
            }


            //draw_position((*inputs).positionW1, (*inputs).positionW2, DevicePosition[BUFFER_LENGTH - 1);

        viewer_endView(); /*OPERATION*/

        draw_position((*inputs).positionW1, (*inputs).positionW2, value);

        sleep(CYCLE_TIME*((*inputs).clockTimeViewer));

    }
    signalC_Viewermodel();
    printf("I'm the viewer thread and I'll die. Countviewer %i \n", count_ModelViewControl[VIEWER]);
    pthread_exit(NULL);
}

void* controller_Function(struct Input *inputs)
{
    FILE* output;

    int index = 0;
    double ThisPosition;

    while(!done) {
        output = fopen ("output.txt","a");

        controller_startControl();

            ThisPosition = DevicePosition[index];

        controller_endControl();

        fprintf(output, "I have moved the device to the position: %f \r\n", ThisPosition);
        index = (index + 1)%BUFFER_LENGTH;
        fclose (output);
        sleep(CYCLE_TIME*((*inputs).clockTimeController));

    }
    signalC_Controllermodel();//salva il caso critico nel cui interface sia bloccato, model non si sveglierebbe dato che interface è bloccato e non può produrre
    printf("I'm the controller thread and I'll die. Count controller %i\n", count_ModelViewControl[CONTROLLER]);
    pthread_exit(NULL);

}

void* listener_Function(sigset_t *set){
    int s, sig;
    char disconnectionNumber[MSG_OUT_LENGTH];
    snprintf(disconnectionNumber, MSG_OUT_LENGTH, "%f", DISCONNECT_NUMBER);
    while(!done) {
        s = sigwait(set, &sig);
        if (s != 0)
            printf("error \n");

        Kill(&new_fd, &server_online, disconnectionNumber);

        /*s = pthread_kill(*server_thread, SIGTSTP);
        //CODICE NON FUNZIONANTE, IDEA:
        //IL SERVER E' BLOCCATO NELL'ATTESA DI RICEVERE UN DATO. NON LEGGERA MAI LA MODIFICA DELLA VARIABILE DONE. DEVO UCCIDERLO BRUTALMENTE
        //SOLUZIONE: DA PROMPT, QUANDO TUTTE LE THREAD HANNO CONCLUSO LA LORO ESECUZIONE MANDO UN CTRL + Z, OPPURE DA CLIENT MANDO UN ULTIMO DATO
        if (s != 0)
            printf("error \n");*/
    }
    printf("I'm the signal listener thread and I'll die \n");
    pthread_exit(NULL);

}


void* server_Function(struct Input *inputs){
    char buf[MSG_IN_LENGTH];  // listen on sock_fd, new connection on new_fd
    int numbytes;


/*      R E C E I V E     M E S S A G E             */

    while (!done){
        double buffer_from_client = 0;
        new_fd = startConnection();
        /*IL PROBLEMA PER LA CHIUSURA DEL PROGRAMMA CHE RIGUARDA IL SERVER (RIMANE BLOCCATO NELL'ATTESA DI UNA CONNESSIONE O UNA RICEZIONE DI MESSAGGIO
        DA PARTE DEL CLIENT POTREBBE ESSERE RISOLTO FACENDO IN MODO CHE IL SERVER CREI UN'ALTRA THREAD DI ASCOLTO, LA QUALE ALLA RICEZIONE DEL SIGINT
        AVVISI IL CLIENT DELLA CHIUSURA DELLA CONNESSIONE (SI ADOPERERA PER FAR SI CHE SI DISCONNETTA ANCH'ESSO) E POI ESEGUI LA DISCONNESSIONE*/

        server_online = CONNECTION_IS_ONLINE;

        while(buffer_from_client !=DISCONNECT_NUMBER){
            if ((numbytes = recv(new_fd, buf, MSG_IN_LENGTH, 0)) == -1) {
                perror("recv");
                pthread_exit(NULL);
            }

            buffer_from_client = atof(buf);
            printf("server: received '%f'\n", buffer_from_client);

            if (buffer_from_client > (*inputs).positionW2){
                buffer_from_client = (*inputs).positionW2;
                printf("The position can't be obtained: new position = %f \n", (*inputs).positionW2);
                if (send(new_fd, "The position can't be obtained \n" , MSG_OUT_LENGTH, 0) == -1)
                    perror("send");
            }

            if (buffer_from_client < (*inputs).positionW1){
                buffer_from_client = (*inputs).positionW1;
                printf("The position can't be obtained: new position = %f \n", (*inputs).positionW1);
                if (send(new_fd, "The position can't be obtained \n", MSG_OUT_LENGTH, 0) == -1)
                    perror("send");
            }
            server_startProducing(new_fd);

            server_Produce(buffer_from_client);
        }

        closeConnection();
        server_online = CONNECTION_IS_OFFLINE;
    }

    printf("I'm the server thread and I'll die \n");
	pthread_exit(NULL);
}
