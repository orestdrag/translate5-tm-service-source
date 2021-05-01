/**
* \file OtmMemoryService.H
*
* \brief Header file for OtmMemoryService 
*
*	Copyright Notice:
*
*	Copyright (C) 2017, QSoft GmbH. All rights reserved
*	
**/
#ifndef _OtmMemoryService_H_
#define _OtmMemoryService_H_

#include "win_types.h"
#include <stdio.h>
#include <functional>

typedef struct _THREADDATA
{
  HANDLE g_ServiceStopEvent;
  HINSTANCE hInstance;
  LPSTR lpCmdLine;
  int nCmdShow;
} THREADDATA, *PTHREADDATA;

struct signal_handler
{
    int signal;
    std::function<void(const int)> handler;
};

BOOL PrepareOtmMemoryService( char *pszService, unsigned *puiPort );
BOOL StartOtmMemoryService(const signal_handler& sh);
void StopOtmMemoryService();
void SetLogFile( FILE *hfLog );
void WriteCrashLog( char *pszLogDir );

#endif
