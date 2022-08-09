#include "ThreadingWrapper.h"
#include <thread>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>


int _getpid(){
    //pthread_id_np_t   tid;
    //tid = pthread_getthreadid_np();
    pid_t id = syscall(__NR_gettid);
    return id;
}