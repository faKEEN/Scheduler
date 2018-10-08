#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

typedef struct value value;
typedef struct noeud noeud;
typedef struct deque deque;

void * newDeque(int taille);
void freeDeque(void *d);
int empty(void *d);

void pushFront(void *d, value v);
void pushBack(void *d, value v);

value popFront(void *d);
value popBack(void *d);
