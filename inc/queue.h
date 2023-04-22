/**
* Module implementing a FIFO queue
* This is a queue, but it also supports removing elements like a linked list.
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
	struct Node* prev;
	struct Node* next;
} Node;

/**
* Queue struct
*/
typedef struct Queue {
	Node* head;
	Node* tail;
	uint32_t size;
	uint32_t virtualSize;
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
* Remove an element pos from the queue
* This function frees the pointer to the node, but not freeing the job in the
* node (the job still needs to be freed manually). The node is not returned,
* be sure to store node somewhere.
*/
void removeQueue(Queue* q, Node* node);

/**
* Free a queue
* This function frees all nodes including the jobs inside.
*/
void freeQueue(Queue* q);

#endif
