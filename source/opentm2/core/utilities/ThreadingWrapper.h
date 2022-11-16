#ifndef _THREADING_WRAPPER_H_
#define _THREADING_WRAPPER_H_

const int MUTEX_ALL_ACCESS = 0x1F0001;
const int INFINITE = -2;

int _getpid();
double getCurrentCPUUsageByProcess();
int getVirtualMemUsageKBValue();

#endif
