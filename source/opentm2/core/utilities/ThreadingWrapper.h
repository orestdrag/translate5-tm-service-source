#ifndef _THREADING_WRAPPER_H_
#define _THREADING_WRAPPER_H_

const int MUTEX_ALL_ACCESS = 0x1F0001;
const int INFINITE = -2;

int _getpid();
double getCurrentCPUUsageByProcess();
int getVirtualMemUsageKBValue();
void mem_usage(double& vm_usage, double& resident_set);


#endif
