#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490"

#define MSG_OUT_LENGTH 50

#define MSG_IN_LENGTH 100

#define DISCONNECT_NUMBER 0.01

bool done = false;
pthread_mutex_t mtx;

/* FUNZIONE CHE RICAVA L'INDIRIZZO DEL SOCKET, CHE SIA ESPRESSO IN IPv4-IPv6   */

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* receive_Function(int* socket){

char buf[MSG_IN_LENGTH];
int numbytes;

while(!done){
    if ((numbytes = recv((*socket), buf, MSG_IN_LENGTH-1, 0)) == -1) {
        perror("recv");
        pthread_exit(NULL);
    }

    buf[numbytes] = '\0';
    printf("'%s' \n",buf);

    if (atof(buf) == DISCONNECT_NUMBER){
        pthread_mutex_lock(&mtx);
            done = true;
        pthread_mutex_unlock(&mtx);
    }
    sleep(1);
}
printf("Thread receive goes out \n");
if (atof(buf) == DISCONNECT_NUMBER){
    sleep(2);
    printf("For the clean termination of the process, please put again the position: \nWhich position do you want?: ");
}

pthread_exit(NULL);

}

void* sendValues_Function(int* socket){

    char msg[MSG_OUT_LENGTH];
    double a = 0;
    sleep(3); //Time to read the 'special number' to use to quit the execution
    while(!done){
        printf("Which position do you want?: ");
        scanf("%le", &a);
        snprintf(msg, MSG_OUT_LENGTH, "%f", a);

        if (send((*socket), msg, MSG_OUT_LENGTH, 0) == -1)
            perror("send");

        if (atof(msg) == DISCONNECT_NUMBER){
            pthread_mutex_lock(&mtx);
                done = true;
            pthread_mutex_unlock(&mtx);
        }

        sleep(1);

    }
    printf("Thread send goes out \n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int status;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

    //VADO A CICLARE TUTTE GLI INDIRIZZI PROPOSTI DA SERVINFO, OVVERO LA LISTA LINKATA NATA DALL'OUTPUT DI "getaddrinfo".
    //IL CICLO SI FERMERA' AL PRIMO INDIRIZZO PUNTATO DA SERVINFO CHE PERMETTE DI CREARE SIA LA SOCKET DEDICATA CHE LA CONNESSIONE

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

    /*INET_NTOP: E' UNA FUNZIONE CHE TRADUCE IN MODO AUTOMATICO L'INDIRIZZO IP*/

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),	s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);
	//E BENE LIBERARE SUBITO LA MEMORIA DINAMICA ALLOCATA. A QUESTO PUNTO INFATTI ABBIAMO SALVATO LOCALMENTE L'INFORMAZIONE UTILE DELLA SOCKET
	//CREATA AL FINE DI COMUNICARE CON IL SERVER (sockfd).

    // {! [shutdown(sockfd, 0)]} --> NON VOGLIO DEGENERARE IL CANALE A MONODIREZIONALE: VOGLIO LASCIARE LA POSSIBILITA AL CLIENT DI RICEVERE MESSAGGI
    //DI ERRORE DAL SERVER. QUESTO SARA' ESEGUITO DA UNA THREAD CLIENT DEDICATA

    printf("Use special number: %f to disconnect comunication \n", DISCONNECT_NUMBER);

    pthread_t sendValues;
    pthread_t receiveMsg;



    status = pthread_create(&sendValues, NULL, (void*)sendValues_Function, &sockfd);
    if (status != 0)
        printf("Error thread sendValues \n");

    status = pthread_create(&receiveMsg, NULL, (void*)receive_Function, &sockfd);
    if (status != 0)
        printf("Error thread receive \n");

    status =  pthread_join(sendValues, NULL);
    if (status != 0)
        printf("Error join thread sendValues \n");

    status =  pthread_join(receiveMsg, NULL);
    if (status != 0)
        printf("Error join thread receive \n");


    pthread_mutex_destroy(&mtx);
    close(sockfd);
    printf("finish\n");
	exit(EXIT_SUCCESS);
}

