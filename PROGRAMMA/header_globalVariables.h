#ifndef header_globalVariables_h
#define header_globalVariables_h

/*                                               G L O B A L      D E F I N E D       V A L U E S                                   */

#define MAX_TUNING_VALUES 20
#define BUFFER_LENGTH 3 //it will define the length of circular arrays
#define MAX_LINE_LENGTH 80
#define VIEWER 0
#define CONTROLLER 1
#define CYCLE_TIME 1

/*                                        S O C K E T      R E Q U I R E D       V A L U E S                                         */

#define PORT "3490"
#define MSG_IN_LENGTH 50
#define BACKLOG 1
#define MSG_OUT_LENGTH 100
#define DISCONNECT_NUMBER 0.01
#define CONNECTION_IS_ONLINE 1
#define CONNECTION_IS_OFFLINE 0

struct Input {
    double positionW1;
    double positionW2;
    double clockTimeViewer;
    double clockTimeController;
};

extern double DeviceInput[BUFFER_LENGTH];
extern double DevicePosition[BUFFER_LENGTH];

extern int count;
extern int count_ModelViewControl[2];

extern int index_model;

extern double serverPosition;

extern bool serverHasProduced;
extern bool done;

extern int new_fd;
extern int server_online;

#endif // header_globalVariables_h
