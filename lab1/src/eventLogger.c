/**
 *  @file	eventLogger.c
 *  @Author	Yuri Tcherezov
 *  @date	March, 2020
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "eventLogger.h"
#include "pa1.h"
#include "common.h"
#include "processHandler.h"

static void Init();
static FILE* file = NULL;


void LogStarted(){
	Init();
	fprintf(file, log_started_fmt, GetLocalProcessId(), getpid(), getppid());
	fflush(file);
	printf(log_started_fmt, GetLocalProcessId(), getpid(), getppid());
}

void LogReceiveAllStarted(){
	Init();
	fprintf(file, log_received_all_started_fmt, GetLocalProcessId());
	fflush(file);
	printf(log_received_all_started_fmt, GetLocalProcessId());
}

void LogDone(){
	Init();
	fprintf(file, log_done_fmt, GetLocalProcessId());
	fflush(file);
	printf(log_done_fmt, GetLocalProcessId());
}

void LogReceiveAllDone(){
	Init();
	fprintf(file, log_received_all_done_fmt, GetLocalProcessId());
	fflush(file);
	printf(log_received_all_done_fmt, GetLocalProcessId());
}

void Init(){
	if (file)
		return;

	file = fopen(events_log, "a");
}

int StrPrintStarted(char* str){
	if (str == NULL)
		return -1;

	return sprintf(str, log_started_fmt, GetLocalProcessId(), getpid(), getppid());
}

int StrPrintReceiveAllStarted(char* str){
	if (str == NULL)
		return -1;

	return sprintf(str, log_received_all_started_fmt, GetLocalProcessId());
}

int StrPrintDone(char* str){
	if (str == NULL)
		return -1;

	return sprintf(str, log_done_fmt, GetLocalProcessId());
}

int StrPrintReceiveAllDone(char* str){
	if (str == NULL)
		return -1;

	return sprintf(str, log_received_all_done_fmt, GetLocalProcessId());
}
