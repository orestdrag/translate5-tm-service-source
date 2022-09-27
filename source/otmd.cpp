#include "RestAPI/OtmMemoryService.h"

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include "opentm2/core/utilities/LogWrapper.h"
#include "cmake/git_version.h"
#include "opentm2/core/utilities/PropertyWrapper.H"
#include "EQF.H"

void handle_interrupt_sig(int sig) {
    LogMessage(TRANSACTION, "Received interrupt signal\n");
    //StopOtmMemoryService();
    LogMessage(TRANSACTION, "Stopped t5memory\n");
}

void service_worker() {
    char szServiceName[80];
    unsigned int uiPort = 0;
    int res = 1;//PrepareOtmMemoryService(szServiceName, &uiPort);
    if (!res) {
        LogMessage(FATAL,"Failed to initialize t5memory");
        std::exit(EXIT_FAILURE);
    }
    LogMessage2(TRANSACTION,"BUILD DATE: ", buildDate);
    LogMessage2(TRANSACTION, "GIT COMMINT INFO: ", gitHash);

    LogMessage2(TRANSACTION, "Version: ", appVersion);
    properties_set_str_anyway(KEY_APP_VERSION, appVersion);
    LogMessage(TRANSACTION, "Initialized t5memory");
    signal_handler sh = { SIGINT, handle_interrupt_sig };
    //res = StartOtmMemoryService(sh);
    if (!res) {
        LogMessage(FATAL,"Failed to start t5memory");
        std::exit(EXIT_FAILURE);
    }
}

int main_test();
int main() {
    bool docker_entrypoint = getpid() == 1;
    if( false && docker_entrypoint ){
        LogMessage(TRANSACTION, "it's docker entrypoint - go to infinite loop");
        while(1){
            sleep(1000);
        };
        LogMessage(TRANSACTION, "shutting down");
    }else{
        LogMessage(TRANSACTION, "Worker thread starting");
        std::thread worker(main_test);
        worker.join();
        LogMessage(TRANSACTION, "Worker thread finished");
    }
}

