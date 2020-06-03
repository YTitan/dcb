#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "ipcHandler.h"
#include "ipc.h"
#include "processHandler.h"
#include "eventLogger.h"
#include "banking.h"
#include "pa2345.h"

IPCHelper* ipc;

/* BEGIN function declarations */
int ChildMain();
void ReceivedDone();
/* END function declarations */

/*BEGIN global process variables declarations*/
balance_t balance;
size_t doneCounter = 0;
int mutexl = 0;
/*END global process variables declarations*/

int main(int argc, char* argv[]){

	int numChildProcesses = 0;
	// parse command line arguments
	int gettingNum = 0;
	for (size_t i = 1; i < argc; ++i){
		if (gettingNum){
			gettingNum = 0;
			numChildProcesses = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "-p") == 0){
			gettingNum = 1;
		}
		else if (strcmp(argv[i], "--mutexl") == 0){
			mutexl = 1;
		}
	}
	// get num processes from command line argument
	if (numChildProcesses <= 0 || numChildProcesses > MAX_NUM_CHILD_PROCESSES){
		printf("usage: %s -p X [--mutexl]\nwhere X is a posiive integer number of child processes less than or equal to %d\n and [--mutexl] is an optional key to enable critical section\n",
			argv[0], MAX_NUM_CHILD_PROCESSES);
		return EXIT_FAILURE;
	}

	// Init logger
	Init();

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
        m.s_header.s_type = STARTED;
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
	
	// do the work
	char str[256];
	int numIter = GetLocalProcessId() * 5;
	for (int i = 1; i <= numIter; ++i){	
		if (mutexl)
			request_cs(ipc);
		
		StrPrintLoopLog(str, i, numIter);
		print(str);

		if (mutexl)
			release_cs(ipc);
	}

	// send done message
	m.s_header.s_type = DONE;
	m.s_header.s_payload_len = StrPrintDone(m.s_payload);
	send_multicast(ipc, &m);
	LogDone();

	// receive messages from other processes
	while (doneCounter < GetNumChildProcesses() - 1){
		int sender = receive_any(ipc, &m);
			
		// act according to message
		switch (m.s_header.s_type){
			case DONE:
				doneCounter++;
				break;
			case CS_REQUEST:
				// send reply message to sender
				m.s_header.s_type = CS_REPLY;
				m.s_header.s_payload_len = 0;
				send(ipc, sender, &m);
				break;
		}

		// check all done
		if (doneCounter == GetNumChildProcesses() - 1){
			break;
		}
	}
	
	// log received all done messages event
	LogReceiveAllDone();

	return EXIT_SUCCESS;
}

void ReceivedDone(){
	doneCounter++; 
}
