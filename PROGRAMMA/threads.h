#ifndef threads_h
#define threads_h

void* interface_Function(double* clockTimeInterface);

void* model_Function(struct Input *inputs);

void* viewer_Function(struct Input *inputs);

void* controller_Function(struct Input *inputs);

void* listener_Function(sigset_t *set);

void* server_Function(struct Input *inputs);

#endif
