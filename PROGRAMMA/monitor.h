#ifndef monitor_h
#define monitor_h

void interface_startProducing();

void interface_endProducing();

void model_startConsuming();

void model_endConsuming();

void model_startProducing();

void model_endProducing();

//void model_getServerPosition(double buffer[], int N);

void viewer_startView();

void viewer_endView();

void controller_startControl();

void controller_endControl();

void Kill(int *socket, int *server_status, char disconnection_msg[]);

void signalC_Viewermodel();

void signalC_Controllermodel();

void signalC_interface();

void server_Produce(double value);

void server_startProducing(int socket);

//void server_endProducing();

double updatePosition(double buffer[], struct Input *input_from_prompt, double actualValue, int N);

void destroy_mutex_condvar();


#endif
