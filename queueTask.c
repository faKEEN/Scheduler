#include <stdio.h>
#include "queueTask.h"

int add(queueTask *q, Task ta) {
     pthread_mutex_lock(&q->mutex);
     if (q == NULL) {
          perror("queue NULL");
          return 1;
     }

     Task *t = malloc(sizeof(Task));
     if (t == NULL) {
          perror("impossible de faire la malloc de la nouvelle Task");
          return 1;
     }

     t->f = ta.f;
     t->argument = ta.argument;
     t->notNone = 1;
     t->next = q->first;
     q->first = t;
     pthread_mutex_unlock(&q->mutex);
     return 0;
}

Task pop(queueTask *q){
     pthread_mutex_lock(&q->mutex);
     if(q== NULL){
          perror("queue vide");
     }

     Task t;
     Task *tt = q->first;
     if(q->first != NULL && q != NULL) {
          t.f = tt->f;
          t.notNone = tt->notNone;
          t.argument = tt->argument;
          q->first = tt->next;
          free(tt);
     }
     else {
          t.notNone = 0;
     }
     pthread_mutex_unlock(&q->mutex);
     return t;
}

void affiche(queueTask *q) {
     if (q == NULL) {
          perror("Pile NULL");
     }
     if(q->first != NULL) {
          Task *newTask = q->first;
          while (newTask->next != NULL) {
               printf("La tache \n");
               newTask = newTask->next;
          }
     }
}
