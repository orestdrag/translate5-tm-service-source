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

int main(void) {
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
    // initialization of OtmMemoryService failed
    if (!res) {
        // TODO write logs, close log file
        exit(EXIT_FAILURE);
    }

    res = StartOtmMemoryService();
    if (!res) {
        // TODO write logs, close log file, deinit
        exit(EXIT_FAILURE);
    }

#ifndef TEMPORARY_COMMENTED
    while (1) {
        // TODO signal to stop
        sleep(1000);
    }
#endif //TEMPORARY_COMMENTED

    StopOtmMemoryService();

    exit(EXIT_SUCCESS);
}
