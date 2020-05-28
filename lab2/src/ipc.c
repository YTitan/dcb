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
	
	size_t m_len = sizeof(msg->s_header) + msg->s_header.s_payload_len;
	size_t total = 0;
	do{
		size_t val = write(ipc->channels[dst].writeDescr, msg, m_len);
		if (val > 0)
			total += val;
	} while (total < m_len);

//	printf("successfully sent a message from process %d to process %d\n", GetLocalProcessId(), dst);

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
	
	
	int ret = 1;
	if (msg->s_header.s_payload_len > 0)
	while ((ret = read(ipc->channels[from].readDescr, msg->s_payload, msg->s_header.s_payload_len)) == -1 && errno == EAGAIN);

	//if (ret == 0)
	//	printf("%d: process %d trying to receive from process %d: eof\n", get_physical_time(), GetLocalProcessId(), from);
	if (ret <= 0)
		return 1;
	else
		return 0;
}


int receive_any(void* self, Message * msg){
	if (self == NULL)
		return 1;
	
	IPCHelper* ipc = (IPCHelper*) self;

	
	int ret = 0;
	// poll each channel for available information
	int proceed = 1;
	while (proceed){
		for (int i = 0; i < ipc->numChannels; i++){
	                if (i != GetLocalProcessId()){
				if (!receive(ipc, i, msg)){
						proceed = 0;
						break;
				}
			}
	        }
	}

	return ret;
}
