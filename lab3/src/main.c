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

IPCHelper* ipc;

/* BEGIN function declarations */
int ChildMain();
/* END function declarations */

/*BEGIN global process variables declarations*/
balance_t balance;
balance_t pending = 0;
/*END global process variables declarations*/

int main(int argc, char* argv[]){

	// get num processes from command line argument
	int numChildProcesses;
	if (argc < 3 || argv[1][0] != '-' || argv[1][1] != 'p' || 
			(numChildProcesses = atoi(argv[2])) <= 0 || numChildProcesses > MAX_NUM_CHILD_PROCESSES){
		printf("usage: %s -p X [balances]\nwhere X is a posiive integer number of child processes less than or equal to %d\n and [balances] is space-separated list of X elements each of wich represents starting balabce of eachprocess\n",
			argv[0], MAX_NUM_CHILD_PROCESSES);
		return EXIT_FAILURE;
	}

	// get starting balances for each process
	balance_t balances[MAX_NUM_CHILD_PROCESSES];
	if (numChildProcesses + 3 > argc){
		printf("list starting balances for all child processes\n");
		return EXIT_FAILURE;
	}
	for (int i = 0; i < numChildProcesses; ++i){
		if (argv[i+3][0] < '0' || argv[i+3][0] > '9'){
			printf("One of the balances is not a number\n");
			return EXIT_FAILURE;
		}
		else {
			balances[i] = atoi(argv[i+3]);
		}
	}
	
	// Init logger
	Init();

	// Create and init child processes
	if (CreateAndInitChildProcesses(numChildProcesses, &ipc, balances) == -1)
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

		bank_robbery(ipc, GetNumChildProcesses());

		// send stop message
		m.s_header.s_type = STOP;
		m.s_header.s_payload_len = 0;
		send_multicast(ipc, &m);


		// receive done message from all child processes
		for (int i = 1; i < ipc->numChannels;){
			if (!receive(ipc, i, &m))
				if (m.s_header.s_type == DONE)
					++i;
		}
		LogReceiveAllDone();

		AllHistory ah;
		ah.s_history_len = GetNumChildProcesses();

		for (int i = 0; i < GetNumChildProcesses();){
			receive(ipc, i + 1, &m);
			if (m.s_header.s_type == BALANCE_HISTORY){
				//printf("((BalanceHistory*)&m.s_payload)->s_history[0].s_balance = %d\n",
				//	((BalanceHistory*)&m.s_payload)->s_history[0].s_balance);
				memcpy(ah.s_history + i, m.s_payload, m.s_header.s_payload_len);
				++i;
			}
		}
		
		print_history(&ah);
		
		WaitForChildProcessesToTerminate();
		return EXIT_SUCCESS;
	}
	else {
		// child process
		return ChildMain();
	}
}

void UpdateBalanceHistory(BalanceHistory* bh);

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

	// init balance history
	BalanceHistory bh;
	bh.s_id = GetLocalProcessId();
	bh.s_history_len = 1;
	bh.s_history[0].s_balance = balance;
	UpdateBalanceHistory(&bh);

	// receive messages from other processes
	size_t doneCounter = 0;
	while (1){
		receive_any(ipc, &m);
			
		// act according to message
		switch (m.s_header.s_type){
			case DONE:
				doneCounter++;
				break;
			case TRANSFER:
				{
					TransferOrder order;
				        memcpy(&order, m.s_payload, sizeof(TransferOrder));
					if (order.s_src == GetLocalProcessId()){
						// transfer money
						balance -= order.s_amount;
						send(ipc, order.s_dst, &m);

						if (GetLocalProcessId() == 1)
							printf("sending transfer at time: %d\n", get_lamport_time());
						
						LogTransferOut(order.s_dst, order.s_amount);
						UpdateBalanceHistory(&bh);
					}
					else if (order.s_dst == GetLocalProcessId()){
						// receive money
						balance += order.s_amount;

						 UpdateBalanceHistory(&bh);
						
						// update pending balance
						for (timestamp_t i = m.s_header.s_local_time; i < get_lamport_time();++i){
	                                                bh.s_history[i].s_balance_pending_in += order.s_amount;
                                                }

						// send ack message to parent
						m.s_header.s_type = ACK;
						m.s_header.s_payload_len = 0;
						send(ipc, PARENT_ID, &m);

						LogTransferIn(order.s_src, order.s_amount);
					}
				}
				break;
			case STOP:
				// send done message
				m.s_header.s_type = DONE;
			        m.s_header.s_payload_len = StrPrintDone(m.s_payload);
			        send_multicast(ipc, &m);
			        // log done event
				LogDone();
				break;
		}

		// check all done
		if (doneCounter == GetNumChildProcesses() - 1){
			break;
		}
	}
	
	// log received all done messages event
	LogReceiveAllDone();

	UpdateBalanceHistory(&bh);

	if (bh.s_history[0].s_balance == 0)
		puts("0");
	// send balance history to parent process
	m.s_header.s_type = BALANCE_HISTORY;
	m.s_header.s_payload_len = sizeof(local_id) + sizeof(uint8_t) + sizeof(BalanceState) * bh.s_history_len;
	memcpy(m.s_payload, &bh, m.s_header.s_payload_len);
	send(ipc, PARENT_ID, &m);

	return EXIT_SUCCESS;
}


void UpdateBalanceHistory(BalanceHistory* bh){
	timestamp_t time = get_lamport_time();
	BalanceState* bs;
	for (timestamp_t i = bh->s_history_len; i < time; ++i){
		bs = &(bh->s_history[i]);	
		bs->s_balance = bh->s_history[i-1].s_balance;
		bs->s_time = i;
		bs->s_balance_pending_in = 0;
	}
	bh->s_history_len = time + 1;	
	bs = &(bh->s_history[time]);
	bs->s_balance = balance;
	bs->s_time = time;
	bs->s_balance_pending_in = 0;
}
