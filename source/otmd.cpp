
//#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "RestAPI/OtmMemoryService.h"

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <folly/portability/GFlags.h>
//#include <glog/logging.h>
#include <execinfo.h>
#include "opentm2/core/utilities/LogWrapper.h"
#include "cmake/git_version.h"
#include "opentm2/core/utilities/PropertyWrapper.H"
#include "EQF.H"

void handle_interrupt_sig(int sig) {
    LogMessage(TRANSACTION, "Received interrupt signal\n");
    //StopOtmMemoryService();
    LogMessage(TRANSACTION, "Stopped t5memory\n");
}

void FailureWriter(const char* data, int size){
    LogMessage(FATAL, data);
    void *array[10];
    size_t sz;

    // get void*'s for all entries on the stack
    sz = backtrace(array, 10);
    std::string backtraceStr;
    for(int i = 0; i < sz && array[i] != 0; i++){
        backtraceStr += (char*) array[i];
        backtraceStr += '\n';

    }
    LogMessage2(FATAL, ":backtrace:\n",  backtraceStr.c_str());

    // print out all the frames to stderr
    //fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}

/*
void CustomPrefix(std::ostream &s, const LogMessageInfo &l, void*) {
   l.
   s << l.severity[0]
   << setw(4) << 1900 + l.time.year()
   << setw(2) << 1 + l.time.month()
   << setw(2) << l.time.day()
   << ' '
   << setw(2) << l.time.hour() << ':'
   << setw(2) << l.time.min()  << ':'
   << setw(2) << l.time.sec() << "."
   << setw(6) << l.time.usec()
   << ' '
   << setfill(' ') << setw(5)
   << l.thread_id << setfill('0')
   << ' '
   << l.filename << ':' << l.line_number << "]";
}
//*/

int proxygen_server_init();
int main(int argc, char* argv[]) {
    FLAGS_log_dir = "/home/or/.t5memory/LOG/";
    FLAGS_colorlogtostderr = true;
    //FLAGS_alsologtostderr = true;
    //google::InstallFailureSignalHandler();
   // google::InstallFailureWriter(FailureWriter);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    //gflags::ParseCommandLineFlags(&argc, &argv, false);
    google::InitGoogleLogging(argv[0]);//, &CustomPrefix);

    //int logLevel = 1/0;
    //WLogMessage(logLevel, "fail");
    //LOG_DEBUG_MSG() << "SOME_DEBUG_MSG";
    
    LogMessage(TRANSACTION, "Worker thread starting");
    std::thread worker(proxygen_server_init);
    worker.join();
    LogMessage(TRANSACTION, "Worker thread finished");
    
}

