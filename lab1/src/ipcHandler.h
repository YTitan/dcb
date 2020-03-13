/**
 * @file ipcHandler.h
 * @author Yuri Tcherezov
 * @date March, 2020
 */

#ifndef __DBC_IPC_HANDLER__H
#define __DBC_IPC_HANDLER__H

#include "ipc.h"
#include "processHandler.h"

typedef struct {
	int readDescr;
	int writeDescr;
} __attribute__ ((packed)) PolyChannel;

typedef struct IPCHelper {
	local_id numChannels;
	PolyChannel channels[MAX_NUM_PROCESSES];
} IPCHelper;

/** Initializes ipc with given channels
 * 
 * @param channels	pointer to array of channels this process can communicate with
 * @param numChannels	number of channels in channels array
 * 
 */
IPCHelper* ipcHelperInit(const PolyChannel* polyChannels, local_id numChannels);

/**
 * Frees memory allocated by ipcHelper
 */
void ipcHelperFree(IPCHelper* ipc);

#endif // __DBC_IPC_HANDLER__H
