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
    LogMessage2(TRANSACTION,"BUILD DATE:", buildDate);
    LogMessage2(TRANSACTION, "GIT COMMINT INFO: ", gitHash);
    LogMessage(TRANSACTION, "Initialized OtmMemoryService");
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

