#include "pa2345.h"
#include "ipc.h"
#include "processHandler.h"
#include "banking.h"

static char forks[MAX_NUM_PROCESSES];
static char dirty[MAX_NUM_PROCESSES];
static char markers[MAX_NUM_PROCESSES];

int try_receive(void * self, local_id from, Message * msg);

void CsInit(){
	for (int i = 1; i <= GetNumChildProcesses(); i++){
		if (i > GetLocalProcessId()){
			// get a dirty fork		
			forks[i] = 1;
			dirty[i] = 1;
		}
		else if (i < GetLocalProcessId()){
			// Get a marker
			markers[i] = 1;
		}
	}
}

void ReceivedDone();

void SendForks(void* ipc){
	Message msg;
	for (int i = 1; i <= GetNumChildProcesses(); ++i){
                if (forks[i] && markers[i] && dirty[i]){
                        msg.s_header.s_type = CS_RELEASE;
                        msg.s_header.s_payload_len = 0;
                        send (ipc, i, &msg);
                        forks[i] = 0;
                        dirty[i] = 0;
                 }
        }
}

void ReceiveAllAvailableMessages(void* ipc){
	Message msg;
	for (int i = 0; i <= GetNumChildProcesses(); i++){
		if (i != GetLocalProcessId()){
			while (!try_receive(ipc, i, &msg)){
				switch (msg.s_header.s_type){
					case CS_REQUEST:
						markers[i] = 1;
						break;
					case CS_RELEASE:
						forks[i] = 1;
						dirty[i] = 0;
						break;
					case DONE:
						ReceivedDone();
						break;
				}
			}
		}
	}
}

int request_cs(const void* self){
	void* ipc = (void*) self;
	Message msg;

	while (1) {

		// check if we can enter critical section
		int numAllowing = 0;
		for (int i = 1; i <= GetNumChildProcesses(); ++i){
			if (forks[i] && (!dirty[i] || !markers[i]))
				numAllowing++;
		}
		if (numAllowing == GetNumChildProcesses() - 1){
			break;
		}

		// try sending markers
		for (int i = 1; i <= GetNumChildProcesses(); ++i){
			if (markers[i] && !forks[i]){
				msg.s_header.s_type = CS_REQUEST;
				msg.s_header.s_payload_len = 0;
				send(ipc, i, &msg);
				markers[i] = 0;
			}
		}

		// try sending forks
		SendForks(ipc);
		
		ReceiveAllAvailableMessages(ipc);		
	}

	// make all forks dirty
	for (int i = 1; i <= GetNumChildProcesses(); ++i){
		if (i != GetLocalProcessId())
			dirty[i] = 1;
	}

	return 0;
}

int release_cs(const void* self){	
	// receive all available messages
	ReceiveAllAvailableMessages((void*) self);
		
	// try sending forks
	SendForks((void*) self);
	return 0;
}
