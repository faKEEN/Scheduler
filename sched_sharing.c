#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "queueTask.h"


void execute_task();
void goSleep();
int threadsAlive();
void * routine();
void execute_task();

typedef struct scheduler scheduler;
struct scheduler{
  int nthreads;
  int activeThreads;
  int qlen;
  queueTask *q;
  pthread_t *threads; // A FREE();
};

scheduler* s;


int sched_init(int nthreads, int qlen, taskfunc f, void *closure){
  s = malloc(sizeof(scheduler));
  if (nthreads != 0) {
       s->nthreads = nthreads;
 }
  else {
       s->nthreads = sched_default_threads();
 }
  s->activeThreads = 0;
  s->qlen = qlen;
  s->q = malloc(sizeof(queueTask));
  s->q->first = NULL;
  pthread_mutex_init(&s->q->mutex,NULL);
  pthread_cond_init(&s->q->cond_wake_up,NULL);
  s->threads = malloc((s->nthreads) * sizeof(pthread_t));
  Task task = {.f=f, .argument=closure};
  add(s->q,task);

  int i;

  for (i = 0; i < s->nthreads; i++) {
       pthread_create(&s->threads[i], NULL,routine,NULL);
 }

 for (i = 0; i < nthreads; i++) {
      pthread_join(s->threads[i],NULL);
}
  return 1;
}



void goSleep() {
          pthread_mutex_lock(&s->q->mutex);
          s->activeThreads--;
          if (s->activeThreads == 0) {
               pthread_cond_signal(&s->q->cond_wake_up);
               pthread_mutex_unlock(&s->q->mutex);
               pthread_exit(NULL);
          }
          else {
               pthread_cond_wait (&s->q->cond_wake_up, &s->q->mutex);
          }
          pthread_mutex_unlock(&s->q->mutex);
}

int threadsAlive() {
     pthread_mutex_lock(&s->q->mutex);
     int bool = s->activeThreads != 0;
     pthread_mutex_unlock(&s->q->mutex);
     return bool;
}

void execute_task(Task t) {
     taskfunc f = t.f;
     void * args = t.argument;
     f(args,s);
}


void * routine() {
     pthread_mutex_lock(&s->q->mutex);
     s->activeThreads++;
     pthread_mutex_unlock(&s->q->mutex);
     while(threadsAlive()){
          Task t = pop(s->q);
          if (t.notNone) {
               execute_task(t);
          }
          else {
               goSleep();
               pthread_mutex_lock(&s->q->mutex);
               s->activeThreads++;
               pthread_mutex_unlock(&s->q->mutex);
          }
     }
     pthread_exit(NULL);
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
     Task task = {.f=f, .argument=closure};
     add(s->q,task);
     pthread_mutex_lock(&s->q->mutex);
     pthread_cond_signal(&s->q->cond_wake_up);
     pthread_mutex_unlock(&s->q->mutex);
     return 1;
}
