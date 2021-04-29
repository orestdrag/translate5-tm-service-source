#include "OtmMemoryService.h"

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>

volatile std::sig_atomic_t stop;

void handle_interrupt_sig(int sig) {
    stop = 1;
    std::cout << "Received interrupt signal\n";
}

int main() {
    std::signal(SIGINT, handle_interrupt_sig);

    char szServiceName[80];
    unsigned int uiPort = 0;

    int res = PrepareOtmMemoryService(szServiceName, &uiPort);
    if (!res) {
        std::cerr << "Failed to initialize OtmMemoryService\n";
        std::exit(EXIT_FAILURE);
    }
    std::cout << "Initialized OtmMemoryService\n";

    res = StartOtmMemoryService();
    if (!res) {
        std::cerr << "Failed to start OtmMemoryService\n";
        std::exit(EXIT_FAILURE);
    }
    std::cout << "Started OtmMemoryService\n";

    std::chrono::seconds time(3);
    do {
        if (stop) {
            StopOtmMemoryService();
            std::cout << "Stopped the service\n";
            std::exit(EXIT_SUCCESS);
        }

        std::cout << "Working...\n";
        std::this_thread::sleep_for(time);
    } while (!stop);
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
