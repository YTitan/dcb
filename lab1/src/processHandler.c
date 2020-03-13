#include <signal.h>
#include "processHandler.h"
#include "ipc.h"
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include "ipcHandler.h"
#include <stdio.h>
#include "common.h"
#include <signal.h>

static local_id localProcessId = -1;

/* BEGIN wait related declarations */
static pid_t childProcessIds[MAX_NUM_CHILD_PROCESSES];
static pid_t GetChildProcessId(local_id localProcessId);
static void SetChildProcessId(local_id localId, pid_t processId);
static local_id numCreatedProcesses = 0;
/* END wait related declarations */

/* BEGIN pipe related private variables and fucntions */
typedef struct {
	int readDescr;
	int writeDescr;
} __attribute__ ((packed)) Pipe;
Pipe pipes[MAX_NUM_PROCESSES * (MAX_NUM_PROCESSES - 1)];
static size_t numCreatedPipes = 0;
/** Creates pipes wich can be used for ipc
 *  @param numProcesses	number of processes to create pipes for 
 *
 *  @returns 0 on success, -1 on error
 */
static int CreatePipes(local_id numProcesses);
static void DestroyCreatedPipes();
/**
 * @param i local id of writer process
 * @param j local id of reader process
 *
 * @returns Pipe in pipes array
 */
static Pipe* GetPipe(local_id i, local_id j);
/* END pipe related private variables and functions */

local_id GetLocalProcessId(){
	return localProcessId;
}

local_id GetNumProcesses(){
	return numCreatedProcesses + 1;
}

local_id CreateAndInitChildProcesses(local_id numChildProcesses, IPCHelper** ipc){
	if (numChildProcesses <1 || numChildProcesses > MAX_NUM_CHILD_PROCESSES)
		return -1;
	local_id numProcesses = numChildProcesses + 1;

	// Open pipes
	if (CreatePipes(numProcesses) != 0){
		// if we were unable to create all the pipes
		// close the ones we created and return
		DestroyCreatedPipes();
		return -1;
	}

	// log pipes
	FILE* f;
	f = fopen(pipes_log, "a");
	fputs("pipes[i][j]: process i writes to this pipe, process j reads from this pipe\nfirst comes read descriptor, then write descriptor\n", f);
	for (int i = 0; i < numProcesses; i++){
		for (int j = 0; j < numProcesses; j++){
			if (i!= j){
				fprintf(f, "pipes[%d][%d] = %d, %d ",
					i, j, GetPipe(i,j)->readDescr, GetPipe(i,j)->writeDescr);
			}
		}
		fputs("\n", f);
	}

	fputs("\nList of channels for each process:\n", f);
	fclose(f);

	localProcessId = 0; // set parent's local process id to 0

	// Create child processes
	for (local_id i = 1; i <= numChildProcesses; ++i) {
		int f = fork();
		if (f == 0) { // in child process
			localProcessId = i;
			break;
		}
		// in parent process
		else if (f == -1) {
			break;
		}
		else {
			++numCreatedProcesses;
			SetChildProcessId(i, f);
		}
	}
	// destroy all the created processes and pipes if failed to create some of them
	if (localProcessId == 0 && numCreatedProcesses != numChildProcesses){
		for (local_id i = 1; i <= numCreatedProcesses; ++i)
			kill(GetChildProcessId(i), SIGKILL);
		WaitForChildProcessesToTerminate();

		DestroyCreatedPipes();
	}

	// at this point begins separate work of each process

	// save channels to communicate with other processes
	// and
	// close pipes that are not needed in current process
	PolyChannel channels[MAX_NUM_PROCESSES];
	for (local_id i = 0; i < numProcesses; ++i) {
		for (local_id j = 0; j < numProcesses; ++j) {
			if (i != j) {
				if (i == localProcessId) {
					channels[j].writeDescr = GetPipe(i, j)->writeDescr;
					close(GetPipe(i, j)->readDescr);
				}
				else if (j == localProcessId) {
					channels[i].readDescr = GetPipe(i, j)->readDescr;
					close(GetPipe(i, j)->writeDescr);
				}
				else {
					close(GetPipe(i, j)->readDescr);
					close(GetPipe(i, j)->writeDescr);
				}
			}
		}
	}

	// log the channels for each process
	f = fopen(pipes_log, "a");
	for (int i = 0; i < numProcesses; ++i){
		if (i != GetLocalProcessId()){
			fprintf(f, "Process %d, channel %d: readDescr = %d, wirteDescr = %d\n",
				GetLocalProcessId(), i, channels[i].readDescr, channels[i].writeDescr);
		}
	}
	fclose(f);

	*ipc = ipcHelperInit(channels, numProcesses);
	return localProcessId;
}

void WaitForChildProcessesToTerminate(){
	if (localProcessId)
		return;
	int wstatus;
	while (numCreatedProcesses)
		waitpid(GetChildProcessId(numCreatedProcesses--), &wstatus, 0);
}

pid_t GetChildProcessId(local_id localProcessId){
	return childProcessIds[localProcessId-1];
}
void SetChildProcessId(local_id localId, pid_t processId){
	childProcessIds[localId-1] = processId;
}

static Pipe* GetPipe(local_id i, local_id j){
	if (i < 0 || i > MAX_NUM_PROCESSES || j < 0 || j > MAX_NUM_PROCESSES || i == j)
		return NULL;
	else
		return pipes + (i * MAX_NUM_PROCESSES + j - ((j>i)?i+1:i));
}

static int CreatePipes(local_id numProcesses){
	for (size_t i = 0; i < numProcesses; i++)
                for (size_t j = 0; j < numProcesses; j++)
                        if (i != j) {
                                if(pipe((int*) GetPipe(i,j)) == -1)
					return -1;
                                else
					++numCreatedPipes;
                        }
	return 0;
}

static void DestroyCreatedPipes(){
	while (numCreatedPipes){
		close(pipes[--numCreatedPipes].readDescr);
		close(pipes[numCreatedPipes].writeDescr);
	}
}
