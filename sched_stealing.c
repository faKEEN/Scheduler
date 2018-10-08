#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sched.h"
#include "deque.h"

typedef struct scheduler scheduler;
struct scheduler{
  int nthreads;
  int nbThreadsOisifs;
  pthread_mutex_t mutexScheduler;
  pthread_mutex_t mutex[20];
  pthread_cond_t cond[20];
  void * deques[20];
  pthread_t * pthreads;
};

struct value{
	taskfunc  function;
	void * arguments;
};

scheduler * s;

int getIndexThread(pthread_t id_pthread){
  int index_pthread = 0;
  while(id_pthread != s->pthreads[index_pthread])
    index_pthread++;
  return index_pthread;
}

deque * getDeque(pthread_t id_pthread){
  return s->deques[getIndexThread(id_pthread)];
}

//fini = 1, on reveille pour le join, on touche pas le nbThreadsOisifs
void reveillerThreads(int fini){
  int i;
  pthread_mutex_lock(&s->mutexScheduler);
  for(i=0; i<s->nthreads; i++){
    pthread_cond_signal(&s->cond[i]);
  }
  if(!fini)
    s->nbThreadsOisifs = 0;
  pthread_mutex_unlock(&s->mutexScheduler);

}

value stealTask(){
  value val;deque * d;
  int index_vole = rand()%(s->nthreads) ;
  d = s->deques[index_vole];
  if((val = popBack(d)).function != NULL){
    return val;
  }
  index_vole = (getIndexThread(pthread_self())+1) % s->nthreads;
  while(index_vole != getIndexThread(pthread_self())){
    d = s->deques[index_vole];
    if((val = popBack(d)).function != NULL){
      return val;
    }
    index_vole = (index_vole+1) % s->nthreads;
  }
  val.function = NULL;
  val.arguments = NULL;
  return val;
}


value doYourTask(deque * d){
  value val;
  if((val = popFront(d)).function != NULL){
    return val;
  }
  val.function = NULL;
  val.arguments = NULL;
  return val;
}

int threadsEncoreActifs(){
  pthread_mutex_lock(&s->mutexScheduler);
  int result = s->nbThreadsOisifs < s->nthreads;
  pthread_mutex_unlock(&s->mutexScheduler);
  return result;
}

int incrementeEtTestThreadsEncoreActifs(){
  pthread_mutex_lock(&s->mutexScheduler);
  s->nbThreadsOisifs += 1;
  int result = s->nbThreadsOisifs < s->nthreads;
  pthread_mutex_unlock(&s->mutexScheduler);
  return result;
}

void patiente(pthread_t id_pthread){
  int index_pthread = getIndexThread(id_pthread);
  if(incrementeEtTestThreadsEncoreActifs()) //on attend les autres threads
    pthread_cond_wait(&s->cond[index_pthread], &s->mutex[index_pthread]);
  else//c'est le dernier a attendre
    reveillerThreads(1);
}


void * run(){
  pthread_t id = pthread_self();
  deque * d = getDeque(id);
  value val;
  //int passage = 0;
  while(threadsEncoreActifs()){ //il reste des threads qui travaillent
    if((val = doYourTask(d) ).function == NULL){ //Si le threads n'a rien a faire
      if((val = stealTask() ).function ==  NULL){ //il tente de voler une tache
        sleep(0.001);
        if((val = stealTask() ).function ==  NULL){
          patiente(id); //il n'a rien a voler
        }
      }
    }
    if(val.function != NULL){ //il a une tache a faire
      //passage++;
      taskfunc func = val.function;
      void * args = val.arguments;
      func(args, s);
    }
  }//printf("fin du thread %u sur la deque %d avec %d passages\n", id, getIndexThread(id), passage);
  pthread_exit(NULL);
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure){
  if(nthreads <= 0)
    nthreads = sched_default_threads();
  s = malloc(sizeof(scheduler));
  s->nthreads = nthreads;
  s->nbThreadsOisifs = 0;
  pthread_mutex_init(&s->mutexScheduler, NULL);
  int i;
  for(i=0; i<nthreads; i++){
    pthread_mutex_init(&s->mutex[i],NULL);
    pthread_cond_init(&s->cond[i],NULL);
    s->deques[i] = newDeque(qlen);
  }
  s->pthreads = malloc(sizeof(pthread_t) * nthreads);
  value val = {.function=f, .arguments=closure};
  pushFront(s->deques[0], val);
  for(i=0; i<nthreads; i++){
    pthread_t * pthread = s->pthreads+i;
    int pc = pthread_create(pthread, NULL, run, NULL);
    if(pc != 0){
      fprintf(stderr, "%s\n", strerror(pc));return EXIT_FAILURE;
    }
  }

  for(i=0; i<nthreads; i++){
    pthread_t * pthread = s->pthreads+i;
    if(pthread_join(*pthread, NULL)){
      perror("pthread join erreur"); return EXIT_FAILURE;
    }
  }
  return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s){
  value val = {.function=f, .arguments=closure};
  pthread_t id_pthread = pthread_self();
  int index_pthread = 0;
  while(id_pthread != s->pthreads[index_pthread])
    index_pthread++;
  pushFront(s->deques[index_pthread], val);
  reveillerThreads(0);
  return 1;
}
