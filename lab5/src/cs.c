#include "pa2345.h"
#include "ipc.h"
#include "processHandler.h"
#include "banking.h"

static char deferredReply[MAX_NUM_PROCESSES];

void ReceivedDone();

int request_cs(const void* self){
	void* ipc = (void*) self;
	// send requst messages
	Message msg;
	msg.s_header.s_type = CS_REQUEST; 
	msg.s_header.s_payload_len = 0;
	send_multicast(ipc, &msg);

	timestamp_t time = get_lamport_time();

	// receive reply messages
	local_id replyCounter = 0;
	while (1) {
		int sender = receive_any(ipc, &msg);
		
		switch (msg.s_header.s_type){ 
			case CS_REQUEST:
				if (time > msg.s_header.s_local_time || 
				    (time == msg.s_header.s_local_time && GetLocalProcessId() < sender)){	
					// send reply message to sender
					msg.s_header.s_type = CS_REPLY;
					msg.s_header.s_payload_len = 0;
					send(ipc, sender, &msg);
				}
				else {
					deferredReply[sender] = 1;
				}
				break;
			case CS_REPLY:
					replyCounter++;
				break;
			case DONE:
				ReceivedDone();
				break;
		}
		
		// check if we can go into cs
		if (replyCounter == GetNumChildProcesses() - 1){
			break;
		}
	}

	return 0;
}

int release_cs(const void* self){	
	Message m;
	m.s_header.s_type = CS_REPLY;
	m.s_header.s_payload_len = 0;
	for (local_id i = 1; i < GetNumChildProcesses()+1; ++i){
		if (deferredReply[i] == 1){
			send((void*) self, i, &m);
			deferredReply[i] = 0;
		}
	}

	return 0;
}
