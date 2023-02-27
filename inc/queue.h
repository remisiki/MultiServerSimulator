/**
* Module implementing a FIFO queue
*/
#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdlib.h>
#include "job.h"

/**
* One Node in a Queue
*/
typedef struct Node {
	Job* job;
	struct Node* next;
} Node;

/**
* Queue struct
*/
typedef struct Queue {
	Node* head;
	Node* tail;
	uint32_t size;
} Queue;

/**
* Init a empty queue
*/
Queue* newQueue();

/**
* Return size of a queue
*/
uint32_t getQueueSize(Queue* q);

/**
* Return whether a queue is empty
* (0 is empty, 1 is non-empty)
*/
uint8_t queueIsEmpty(Queue* q);

/**
* Push a job to the head of the queue
*/
void pushQueue(Queue* q, Job* job);

/**
* Pop the head of a queue
* This function frees the pointer to the head node, but not freeing the job in
* the node (the job still needs to be freed manually). The head node is not
* returned, you may need to store q->head before pop.
*/
void popQueue(Queue* q);

/**
* Free a queue
* This function frees all nodes including the jobs inside.
*/
void freeQueue(Queue* q);

#endif
