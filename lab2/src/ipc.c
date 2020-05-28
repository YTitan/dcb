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

	//printf("successfully sent a message from process %d to process %d\n", GetLocalProcessId(), dst);

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


static int ReadNumBytesOrNothing(IPCHelper* ipc, local_id src, void* buf, size_t numBytes){
	int val = read(ipc->channels[src].readDescr, buf, numBytes);

	//if (val > 0)
	//	printf("process %d trying to get data from process %d: val = %d\n", GetLocalProcessId(), src, val);

	if (val <= 0) return 0;


	if (val == numBytes) return 1;

	size_t total = val;
	while (total < numBytes) {
		val = read(ipc->channels[src].readDescr, buf, numBytes - total);
		if (val > 0)
			total += val;
	}

	return 1;
}

int receive_any(void* self, Message * msg){
	if (self == NULL)
		return 1;
	
	IPCHelper* ipc = (IPCHelper*) self;

	// make every read end of the pipe non-blocking
	int* flags = (int*) malloc(sizeof(int) * ipc->numChannels);
	for (int i = 0; i < ipc->numChannels; i++){
		if (i != GetLocalProcessId()){
			flags[i] = fcntl(ipc->channels[i].readDescr, F_GETFL);
			fcntl(ipc->channels[i].readDescr, F_SETFL, flags[i] | O_NONBLOCK);
	//		perror("making non-blocking");
		}
	}
	
	int ret = 0;
	// poll each channel for available information
	int proceed = 1;
	while (proceed){
		for (int i = 0; i < ipc->numChannels; i++){
	                if (i != GetLocalProcessId()){
				if (ReadNumBytesOrNothing(ipc, i, &msg->s_header, sizeof(msg->s_header))){
	//				printf("process %d received message from process %d\n", GetLocalProcessId(), i);
					ReadNumBytesOrNothing(ipc, i, &msg->s_payload, msg->s_header.s_payload_len);
					proceed = 0;
					break;
				}

			}
	        }
	}


	// make io blocking again
	for (int i = 0; i < ipc->numChannels; i++){
		if (i != GetLocalProcessId()){
                        fcntl(ipc->channels[i].readDescr, F_SETFL, flags[i]);
                }
        }
	free(flags);

	return ret;
}
