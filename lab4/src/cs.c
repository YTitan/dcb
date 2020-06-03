#include "pa2345.h"
#include "ipc.h"
#include "processHandler.h"
#include "queue.h"
#include "banking.h"

extern Queue csQueue;

void ReceivedDone();

int request_cs(const void* self){
	void* ipc = (void*) self;
	// send requst messages
	Message msg;
	msg.s_header.s_type = CS_REQUEST; 
	msg.s_header.s_payload_len = 0;
	send_multicast(ipc, &msg);

	QItem i;
	i.processId = GetLocalProcessId();
	i.time = get_lamport_time();
	QueueInsert(&csQueue, i);

	// receive reply messages
	local_id replyCounter = 0;
	while (1) {
		int sender = receive_any(ipc, &msg);
		
		switch (msg.s_header.s_type){ 
			case CS_REQUEST:
				// put the request into the queue
				i.processId = sender;
				i.time = msg.s_header.s_local_time;
				QueueInsert(&csQueue, i);
				
				// send reply message to sender
				msg.s_header.s_type = CS_REPLY;
				msg.s_header.s_payload_len = 0;
				send(ipc, sender, &msg);
				break;
			case CS_REPLY:
				// check if the message's time is greater than this request
				if (msg.s_header.s_local_time > i.time)
					replyCounter++;
				break;
			case CS_RELEASE:
					// remove the request from queue
					QueueRemoveByProcessId(&csQueue, sender);
				break;
			case DONE:
				ReceivedDone();
				break;
		}
		
		// check if we can go into cs
		if ((replyCounter == GetNumChildProcesses() - 1) &&
				QueueGetHighestPriority(&csQueue).processId == GetLocalProcessId()){
			break;
		}
	}

	return 0;
}

int release_cs(const void* self){	
	// remove this processes request from local queue
	QueueRemoveByProcessId(&csQueue, GetLocalProcessId());

	// send message to other processes to remove the request from their local queues
	Message m;
	m.s_header.s_type = CS_RELEASE;
	m.s_header.s_payload_len = 0;
	send_multicast((void*) self, &m);

	return 0;
}
