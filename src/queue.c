#include "queue.h"

Queue* newQueue() {
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	return q;
}

uint32_t getQueueSize(Queue* q) {
	return q->size;
}

uint8_t queueIsEmpty(Queue* q) {
	return (q->size == 0);
}

void pushQueue(Queue* q, Job* job) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->job = job;
	node->next = NULL;
	if (queueIsEmpty(q)) {
		q->head = node;
		q->tail = node;
		q->size = 1;
	} else {
		q->tail->next = node;
		q->tail = node;
		q->size ++;
	}
}

void popQueue(Queue* q) {
	if (!queueIsEmpty(q)) {
		Node* top = q->head;
		q->head = q->head->next;
		free(top);
		q->size --;
		if (q->head == NULL) {
			q->tail = NULL;
		}
	}
}

void freeQueue(Queue* q) {
	while (q->head != NULL) {
		Node* top = q->head;
		q->head = q->head->next;
		free(top->job);
		free(top);
	}
	free(q);
}
