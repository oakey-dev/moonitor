#ifndef _COLLECTOR_H_
#define _COLLECTOR_H_

#include <semaphore.h>

struct plugin {
   int interval;
   sem_t* lock;
   int (*cmd)();
};

struct queue {
   time_t timestamp;
   struct plugin** plugins; // NULL terminating
   struct queue* next;
};

void* collector_thread(void* arg);
struct queue* queue_add(struct queue* que, struct plugin* p, time_t time);

#endif
