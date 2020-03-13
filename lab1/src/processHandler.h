/**
 * @file	processHandler.h
 * @author	Yuri Tcherezov
 * @date	March, 2020
 * @brief	Lab1 helper
 */

#ifndef __DBC_PROCESS_HANDLER__H
#define __DBC_PROCESS_HANDLER__H

#include "ipc.h"

struct IPCHelper;

enum {
	MAX_NUM_CHILD_PROCESSES = 10,
	MAX_NUM_PROCESSES
};

/**
 * @returns local id of the current process
 */
local_id GetLocalProcessId();

local_id GetNumProcesses();

/** Create numChildProcesses number of child processes and initialize ipc between them (and the parent process)
 *  This function returns in all of the created processes and the parent
 *  the only difference being the return value and
 *  the return value of GetLocalProcessId()
 *
 *  if an error occurs all created processes are destroyed and all created pipes are closed
 *
 *  @param numChildProcesses	Number of chil processes to create; Cannot exceed MAX_NUM_CHILD_PROCESSES
 *
 *  @return local id of a current process (parent's id is 0) or -1 if an error occurs
 */
local_id CreateAndInitChildProcesses(local_id numChildProcesses, struct IPCHelper** ipc);

/** Waits untill child processes are turminated
 *  Only has effect when called by the parent process
 *  Does nothing if called by a child process
 */
void WaitForChildProcessesToTerminate();

#endif //__DBC_PROCESS_HANDLER__H
