#include "queue.h"


int QItemComparator(QItem i1, QItem i2){
	int res = i2.time - i1.time;
	if (res == 0)
		res = i1.processId - i2.processId;
	return res;
}

void InitQueue(Queue* q){
	if (q == NULL)
		return;

	q->numItems = 0;
}

void QueueInsert(Queue* q, QItem i){
	q->items[q->numItems++] = i;
}

QItem QueueGetHighestPriority(Queue* q){
	QItem res;
	if (q->numItems == 0){
		res.processId = -1;
	}
	else {
		res = q->items[0];
		for (size_t i = 1; i < q->numItems; ++i){
			if (QItemComparator(q->items[i], res) > 0)
				res = q->items[i];
		}
	}

	return res;
}

static void QueueRemove(Queue* q, size_t index){
	if (index >= q->numItems)
		return;

	q->numItems--;

	for (size_t i = index; i < q->numItems; i++){
		q->items[i] = q->items[i+1];
	}
}

QItem QueueRemoveHighestPriority(Queue* q){
	QItem res;
	if (q->numItems == 0){
		res.processId = -1;
	}
	else {
		res = q->items[0];
		size_t index = 0;
		for (size_t i = 1; i < q->numItems; ++i){
			if (QItemComparator(q->items[i], res) > 0){
				res = q->items[i];
				index = i;
			}
		}
		QueueRemove(q, index);
	}
	return res;
}	

void QueueRemoveByProcessId(Queue* q, local_id processId){
	size_t index = -1;
	for (size_t i = 0; i < q->numItems; ++i){
		if (q->items[i].processId == processId){
			index = i;
			break;
		}
	}	

	QueueRemove(q, index);
}
