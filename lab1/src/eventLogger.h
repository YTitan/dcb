#ifndef __DCB_EVENT_LOGGER__H
#define __DCB_EVENT_LOGGER__H

void LogStarted();

void LogReceiveAllStarted();

void LogDone();

void LogReceiveAllDone();

int StrPrintStarted(char* str);

int StrPrintReceiveAllStarted(char* str);

int StrPrintDone(char* str);

int StrPrintReceiveAllDone(char* str);

#endif //__DCB_EVENT_LOGGER__H
