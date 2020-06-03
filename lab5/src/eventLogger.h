#ifndef __DCB_EVENT_LOGGER__H
#define __DCB_EVENT_LOGGER__H

#include "banking.h"
#include "ipc.h"

void Init();

void LogStarted();

void LogReceiveAllStarted();

void LogDone();

void LogReceiveAllDone();

void LogTransferOut(local_id receiverId, balance_t amount);

void LogTransferIn(local_id senderId, balance_t amount);

int StrPrintStarted(char* str);

int StrPrintReceiveAllStarted(char* str);

int StrPrintDone(char* str);

int StrPrintReceiveAllDone(char* str);

int StrPrintLoopLog(char* str, int i, int numIter);

#endif //__DCB_EVENT_LOGGER__H
