#include "ipc.h"
#include "ipcHandler.h"
#include "processHandler.h"
#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

IPCHelper* ipcHelperInit(const PolyChannel* polyChannels, local_id numChannels){
	if (numChannels < 0 || numChannels > MAX_NUM_PROCESSES)
		return NULL;
	IPCHelper* ipc = (IPCHelper*) malloc(sizeof(IPCHelper));
	ipc->numChannels = numChannels;
	memcpy(ipc->channels, polyChannels, sizeof(PolyChannel) * numChannels);
	return ipc;
}

void ipcHelperFree(IPCHelper* ipc){
	free(ipc);
}

int send(void * self, local_id dst, const Message * msg){
	if (self == NULL)
		return 1;
	IPCHelper* ipc = (IPCHelper*) self;
	if (dst > ipc->numChannels || dst < 0)
		return 1;
	
	if (write(ipc->channels[dst].writeDescr, msg, sizeof(msg->s_header) + msg->s_header.s_payload_len) == -1)
		return 1;

	return 0;
}

int send_multicast(void * self, const Message * msg){
	if (self == NULL)
                return 1;
        IPCHelper* ipc = (IPCHelper*) self;
	
	for (local_id i = 0; i < ipc->numChannels; ++i)
		if (i != GetLocalProcessId())
			if (send(self, i, msg))
				return 1;
	return 0;
}

int receive(void * self, local_id from, Message * msg){
	if (self == NULL)
                return 1;
        IPCHelper* ipc = (IPCHelper*) self;
	
	if (from > ipc->numChannels || from < 0)
		return 1;

	if (read(ipc->channels[from].readDescr, &msg->s_header, sizeof(msg->s_header)) == -1)
		return 1;
	
	if (read(ipc->channels[from].readDescr, msg->s_payload, msg->s_header.s_payload_len) == -1)
		return 1;

	return 0;
}

int receive_any(void* self, Message * msg){
	if (self == NULL)
		return 1;
	
	IPCHelper* ipc = (IPCHelper*) self;

	// make every read end of the pipe non-blocking
	int* flags = (int*) malloc(sizeof(int) * ipc->numChannels);
	for (int i = 0; i < ipc->numChannels; i++){
		if (i != GetLocalProcessId()){
			flags[i] = fcntl(ipc->channels[i].readDescr, F_GETFD);
			fcntl(ipc->channels[i].readDescr, F_SETFD, flags[i] | O_NONBLOCK);
		}
	}

	// poll each channel for available information
	int proceed = 1;
	while (proceed){
		for (int i = 0; i < ipc->numChannels; i++){
	                if (i != GetLocalProcessId()){
	                        if (receive(self, i, msg)){
					if (errno != EAGAIN)
						return 1;
				}
	                }
			else {
				proceed = 0;
				break;
			}
	        }
	}


	// make io blocking again
	for (int i = 0; i < ipc->numChannels; i++){
		if (i != GetLocalProcessId()){
                        fcntl(ipc->channels[i].readDescr, F_SETFD, flags[i]);
                }
        }
	free(flags);

	return 0;
}
