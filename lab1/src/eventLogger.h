/**
 *  @file	eventLogger.h
 *  @Author	Yuri Tcherezov
 *  @date	March, 2020
 *  @vrief	A set of helper function to help logging events
 */

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
