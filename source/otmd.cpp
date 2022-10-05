#include "RestAPI/OtmMemoryService.h"

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <folly/portability/GFlags.h>
#include <glog/logging.h>
#include "opentm2/core/utilities/LogWrapper.h"
#include "cmake/git_version.h"
#include "opentm2/core/utilities/PropertyWrapper.H"
#include "EQF.H"

void handle_interrupt_sig(int sig) {
    LogMessage(TRANSACTION, "Received interrupt signal\n");
    //StopOtmMemoryService();
    LogMessage(TRANSACTION, "Stopped t5memory\n");
}

int proxygen_server_init();
int main(int argc, char* argv[]) {
    FLAGS_log_dir = "/home/or/.t5memory/LOG/";
    //FLAGS_alsologtostderr = 1;

    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    
    LogMessage(TRANSACTION, "Worker thread starting");
    std::thread worker(proxygen_server_init);
    worker.join();
    LogMessage(TRANSACTION, "Worker thread finished");
    
}

