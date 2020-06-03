#ifndef QUEUE_H
#define QUEUE_H

#include "ipc.h"
#include "processHandler.h"

typedef struct {
	        local_id processId;
	        timestamp_t time;
} QItem;

// returns >0 if item1 has grater priority, 0 if they prorities are equal, <0 if item2 has greate priority
int QItemComparator (QItem i1, QItem i2);

typedef struct {
	        QItem items[MAX_NUM_CHILD_PROCESSES];
		size_t numItems;
} Queue;

void QueueInit(Queue* q);

void QueueInsert(Queue* q, QItem i);

QItem QueueGetHighestPriority(Queue* q);

QItem QueueRemoveHighestPriority(Queue* q);

void QueueRemoveByProcessId(Queue* q, local_id processId);

#endif //QUEUE_H
