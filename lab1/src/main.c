#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "ipcHandler.h"
#include "ipc.h"
#include "processHandler.h"
#include "eventLogger.h"

IPCHelper* ipc;

/* BEGIN function declarations */
int ChildMain();
/* END function declarations */

int main(int argc, char* argv[]){

	// get num processes a command line argument
	int numChildProcesses;
	if (argc < 3 || argv[1][0] != '-' || argv[1][1] != 'x' || 
			(numChildProcesses = atoi(argv[2])) <= 0 || numChildProcesses > MAX_NUM_CHILD_PROCESSES){
		printf("usage: pa1 -x P\nwhere P is a posiive integer of child processes less than or equal to %d\n", MAX_NUM_CHILD_PROCESSES);
		return EXIT_FAILURE;
	}

	// Create and init child processes
	if (CreateAndInitChildProcesses(numChildProcesses, &ipc) == -1)
		return EXIT_FAILURE;


	if (GetLocalProcessId() == 0){
		// parent process
		// receive started message from child processes
		Message m;
		for (int i = 1; i < ipc->numChannels;){
			int ret = receive(ipc, i, &m);
		        if (!ret && m.s_header.s_type == STARTED){
				i++;
			}
		}
		LogReceiveAllStarted();

		// receive done message from all child processes
		for (int i = 1; i < ipc->numChannels;){
			if (!receive(ipc, i, &m))
				if (m.s_header.s_type == DONE)
					++i;
		}
		LogReceiveAllDone();
		
		WaitForChildProcessesToTerminate();
		return EXIT_SUCCESS;
	}
	else {
		// child process
		return ChildMain();
	}
}


int ChildMain(){
	// log started event
	LogStarted();

	// send started message
	Message m;
	m.s_header.s_magic = MESSAGE_MAGIC;
        m.s_header.s_type = STARTED;
	m.s_header.s_local_time = time(NULL);
	m.s_header.s_payload_len = StrPrintStarted(m.s_payload);
	send_multicast(ipc, &m);
	
	// receive started message from all other child processes
	for (int i = 1; i < ipc->numChannels;)
		if (i != GetLocalProcessId()){	
			int ret = receive(ipc, i, &m);
			if (!ret && m.s_header.s_type == STARTED)
				i++;
		}	
		else
			++i;
	// log received all started messages
	LogReceiveAllStarted();

	// send done message
	m.s_header.s_type = DONE;
	m.s_header.s_local_time = time(NULL);
	m.s_header.s_payload_len = StrPrintDone(m.s_payload);
	send_multicast(ipc, &m);

	// log done event
	LogDone();

	// receive done messages from all other child processes
	for (int i = 1; i < ipc->numChannels;)
		if (i != GetLocalProcessId()){
			int ret = receive(ipc, i, &m);
			if (!ret && m.s_header.s_type == DONE)
				i++;
		}
		else
			++i;

	// log received all done messages event
	LogReceiveAllDone();

	return EXIT_SUCCESS;
}
