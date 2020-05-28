#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "eventLogger.h"
#include "pa2345.h"
#include "common.h"
#include "processHandler.h"
#include "banking.h"

extern balance_t balance;

static FILE* file = NULL;


void LogStarted(){
	Init();
	fprintf(file, log_started_fmt, get_physical_time(), GetLocalProcessId(), getpid(), getppid(), balance);
	fflush(file);
	printf(log_started_fmt, get_physical_time(), GetLocalProcessId(), getpid(), getppid(), balance);
}

void LogReceiveAllStarted(){
	Init();
	fprintf(file, log_received_all_started_fmt, get_physical_time(), GetLocalProcessId());
	fflush(file);
	printf(log_received_all_started_fmt, get_physical_time(), GetLocalProcessId());
}

void LogDone(){
	Init();
	fprintf(file, log_done_fmt, get_physical_time(), GetLocalProcessId(), balance);
	fflush(file);
	printf(log_done_fmt, get_physical_time(), GetLocalProcessId(), balance);
}

void LogReceiveAllDone(){
	Init();
	fprintf(file, log_received_all_done_fmt, get_physical_time(), GetLocalProcessId());
	fflush(file);
	printf(log_received_all_done_fmt, get_physical_time(), GetLocalProcessId());
}

void LogTransferOut(local_id receiverId, balance_t amount){
	Init();
	fprintf(file, log_transfer_out_fmt, get_physical_time(), GetLocalProcessId(), amount, receiverId);
	fflush(file);
	printf(log_transfer_out_fmt, get_physical_time(), GetLocalProcessId(), amount, receiverId);
}

void LogTransferIn(local_id senderId, balance_t amount){
	Init();
        fprintf(file, log_transfer_in_fmt, get_physical_time(), GetLocalProcessId(), amount, senderId);
        fflush(file);
        printf(log_transfer_in_fmt, get_physical_time(), GetLocalProcessId(), amount, senderId);
}

void Init(){
	if (file)
		return;

	file = fopen(events_log, "a");
}

int StrPrintStarted(char* str){
	if (str == NULL)
		return -1;

	return sprintf(str, log_started_fmt, get_physical_time(), GetLocalProcessId(), getpid(), getppid(), balance);
}

int StrPrintDone(char* str){
	if (str == NULL)
		return -1;

	return sprintf(str, log_done_fmt, get_physical_time(), GetLocalProcessId(), balance);
}
