#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "deque.h"
#include "sched.h"

#define FIRST_POSITION 3

struct value{
	taskfunc function;
	void * arguments;
};

struct deque {
  pthread_mutex_t mutex;
	int taille;
	int pointeurFront;
	int pointeurBack;
};

void avancerPointeurFront(deque * deque){
	deque->pointeurFront++;
	if(deque->pointeurFront == 0 || deque->pointeurFront==deque->taille){
		deque->pointeurFront = FIRST_POSITION;
	}
}

void avancerPointeurBack(deque * deque){
	deque->pointeurBack--;
	if(deque->pointeurBack < FIRST_POSITION){
		deque->pointeurBack = deque->taille-1;
	}
}

void reculerPointeurFront(deque * deque){
	deque->pointeurFront--;
	if(deque->pointeurFront < FIRST_POSITION){
		deque->pointeurFront = deque->taille-1;
	}

}

void reculerPointeurBack(deque * deque){
	deque->pointeurBack++;
	if(deque->pointeurBack==deque->taille){
		deque->pointeurBack = FIRST_POSITION;
	}

}

void * newDeque(int taille) {
	void * d = mmap(NULL, sizeof(deque) + sizeof(value)*taille, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	if (d == MAP_FAILED){
		perror("erreur de mmap");exit(0);
	}
	deque * deque = d;
	deque->taille = taille + FIRST_POSITION;
	deque->pointeurFront = deque->pointeurBack = -1;
	pthread_mutex_init(&deque->mutex,NULL);
	return d;
}

void freeDeque(void *d) {
	deque * deque = d;
	munmap(d, sizeof(deque) + sizeof(value) * deque->taille);
}

int empty(void *d) {
	deque * deque = d;
	return deque->pointeurFront == -1;
}

void pushFront(void *d, value v) {
	deque * deque = d;
	pthread_mutex_lock(&deque->mutex);
	avancerPointeurFront(deque);
	value * val = d+sizeof(deque) + deque->pointeurFront * sizeof(value);
	*val = v;
	if (deque->pointeurBack == -1) {
		deque->pointeurBack = deque->pointeurFront;
	}
	pthread_mutex_unlock(&deque->mutex);
}

void pushBack(void *d, value v) {
	deque * deque = d;
	pthread_mutex_lock(&deque->mutex);
	avancerPointeurBack(deque);
	value * val = d+sizeof(deque) + deque->pointeurBack * sizeof(value);
	*val = v;
	if (deque->pointeurFront == -1) {
		deque->pointeurFront = deque->pointeurBack;
	}
	pthread_mutex_unlock(&deque->mutex);
}

value popFront(void *d) {
	deque * deque = d;
	pthread_mutex_lock(&deque->mutex);
	if(empty(d)){
		value val = {.function = NULL, .arguments = NULL};
		pthread_mutex_unlock(&deque->mutex);
		return val;
	}
	value * v = d+sizeof(deque) + deque->pointeurFront * sizeof(value);
	if (deque->pointeurFront == deque->pointeurBack)
		deque->pointeurFront = deque->pointeurBack = -1;
	else
		reculerPointeurFront(deque);
	pthread_mutex_unlock(&deque->mutex);
	return *v;
}

value popBack(void *d) {
	deque * deque = d;
	pthread_mutex_lock(&deque->mutex);
	if(empty(d)){
		value val = {.function = NULL, .arguments = NULL};
		pthread_mutex_unlock(&deque->mutex);
		return val;
	}
	value * v = d+sizeof(deque) + deque->pointeurBack * sizeof(value);
	if (deque->pointeurFront == deque->pointeurBack)
		deque->pointeurFront = deque->pointeurBack = -1;
	else
		reculerPointeurBack(deque);
	pthread_mutex_unlock(&deque->mutex);
	return *v;
}
