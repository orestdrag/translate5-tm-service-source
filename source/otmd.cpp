#include "OtmMemoryService.h"

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>
#include "opentm2/core/utilities/LogWrapper.h"
#include "cmake/git_version.h"

void handle_interrupt_sig(int sig) {
    LogMessage(INFO, "Received interrupt signal\n");
    StopOtmMemoryService();
    LogMessage(INFO, "Stopped OtmMemoryService\n");
}

void service_worker() {
    char szServiceName[80];
    unsigned int uiPort = 0;
    int res = PrepareOtmMemoryService(szServiceName, &uiPort);
    if (!res) {
        LogMessage(FATAL,"Failed to initialize OtmMemoryService");
        std::exit(EXIT_FAILURE);
    }
    //LogMessage2(INFO, "Git hash: ", gitHash);
    LogMessage2(WARNING,"BUILD DATE:", buildDate);
    LogMessage2(WARNING, "GIT COMMINT INFO: ", gitHash);
    LogMessage(INFO, "Initialized OtmMemoryService");
    signal_handler sh = { SIGINT, handle_interrupt_sig };
    res = StartOtmMemoryService(sh);
    if (!res) {
        LogMessage(FATAL,"Failed to start OtmMemoryService");
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    LogMessage(INFO, "Worker thread starting");
    std::thread worker(service_worker);
    worker.join();
    LogMessage(INFO, "Worker thread finished");
}



// TODO daemon
#ifdef TEMPORARY_COMMENTED

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "OtmMemoryService.h"

int main() {
    /* process ID, session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        exit(EXIT_FAILURE);
    }

    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
        printf("Fork succeded, exiting parent process\n");
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);
                
    /* TODO Open logs here */  

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        printf("Failed to create new session\n");
        exit(EXIT_FAILURE);
    }

     /* Change the current working directory */
    if ((chdir("/")) < 0) {
        printf("Failed to change the directory\n");
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* OtmMemoryService */
    char szServiceName[80];
    unsigned int uiPort = 0;

    int res = PrepareOtmMemoryService(szServiceName, &uiPort);
    if (!res) {
        // TODO write logs, close log file
        exit(EXIT_FAILURE);
    }

    res = StartOtmMemoryService();
    if (!res) {
        // TODO write logs, close log file, deinit
        exit(EXIT_FAILURE);
    }

    while (1) {
        sleep(1000);
    }

    StopOtmMemoryService();

    exit(EXIT_SUCCESS);
}

#endif //TEMPORARY_COMMENTED
