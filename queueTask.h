#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "sched.h"

typedef struct Task Task;
struct Task {
     taskfunc f;
     void * argument;
     Task *next;
     int notNone; /* 0 -> false, 1 -> true */
};

typedef struct queueTask queueTask;
struct queueTask {
     Task *first;
     pthread_cond_t cond_wake_up;
     pthread_mutex_t mutex;
};

int isEmpty(queueTask *q);
int add(queueTask* q, Task t);
Task pop(queueTask *q);
void affiche(queueTask *q);
