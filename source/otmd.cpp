
//#define GLOG_NO_ABBREVIATED_SEVERITIES


#include <iostream>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <folly/portability/GFlags.h>

#include <execinfo.h>
#include <csignal>
#include "RestAPI/OtmMemoryServiceWorker.h"
#include "opentm2/core/utilities/LogWrapper.h"
#include "opentm2/core/utilities/FilesystemHelper.h"
#include "cmake/git_version.h"
#include "opentm2/core/utilities/Property.h"
#include "opentm2/core/utilities/EncodingHelper.h"
#include "EQF.H"



static bool ValidatePort(const char* flagname, int32_t value) {
   if (value > 0 && value < 32768)   // value is ok
     return true;
   printf("Invalid value for --%s: %d\n", flagname, (int)value);
   return false;
}

DEFINE_int32(port, 4080, "What port to listen on");
DEFINE_validator(port, &ValidatePort);

DEFINE_string(t5_ip, "", "Which ip to use in t5memory(default is any). Should be in format '1.1.1.1', default is to listen to all available ip");


DEFINE_string(servicename, "t5memory", "Sets service name to use in url");

static bool ValidateTriplesThreshold(const char* flagname, int32_t value) {
   if (value >= 0 && value <= 100)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [0...100]\n", flagname, (int)value);
   return false;
}

DEFINE_int32(triplesthreshold, 5, "Sets threshold to pre fuzzy filtering based on hashes of neibour tokens");
DEFINE_validator(triplesthreshold, &ValidateTriplesThreshold);


static bool ValidateTimeout(const char* flagname, int32_t value) {
   if (value >= 0 && value <= 3600000)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [0...3600000]\n", flagname, (int)value);
   return false;
}

DEFINE_int32(timeout, 180000, "Sets timeout for service request handling");
DEFINE_validator(timeout, &ValidateTimeout);


static bool ValidateThreads(const char* flagname, int32_t value) {
   if (value >= 1 && value <= 100)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [1...100]\n", flagname, (int)value);
   return false;
}

DEFINE_int32(servicethreads, 1, "Sets amought of worker threads for service");
DEFINE_validator(servicethreads, &ValidateThreads);


static bool ValidateRAM(const char* flagname, int32_t value) {
   if (value >= 1 && value <= 10000)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [1...10000]\n", flagname, (int)value);
   return false;
}

DEFINE_int32(allowedram, 2048, "Sets amought RAM(in MB) allowed for service to use");
DEFINE_validator(allowedram, &ValidateRAM);

static bool ValidateLOGlevel(const char* flagname, int32_t value) {
   if (value >= 0 && value <= 6)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [0...6]\n", flagname, (int)value);
   return false;
}

DEFINE_int32(t5loglevel, T5INFO, "Sets t5memory log level threshold from DEVELOP(0) to TRANSACTION(6)");
DEFINE_validator(t5loglevel, &ValidateLOGlevel);


DEFINE_bool(useconfigfile, false, "Set to use values from config file that should be located under ~/.t5memory/t5memory.conf");


DEFINE_bool(forbiddeletefiles, false, "Set to true to keep all files(including temporary and tm)");

void handle_interrupt_sig(int sig) {
    T5LOG( T5TRANSACTION) << "Received interrupt signal\n";
    //StopOtmMemoryService();
    T5LOG( T5TRANSACTION) << "Stopping t5memory\n";

}

void FailureWriter(const char* data, int size){
    T5LOG( T5FATAL) << data;
    void *array[10];

    // get void*'s for all entries on the stack
    size_t sz = backtrace(array, 10);
    std::string backtraceStr;
    for(int i = 0; i < sz && array[i] != 0; i++){
        backtraceStr += (char*) array[i];
        backtraceStr += '\n';
    }
    T5LOG( T5FATAL) << ":backtrace:\n" << backtraceStr ;

    // print out all the frames to stderr
    //fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}


namespace
{
  volatile std::sig_atomic_t gSignalStatus;
}
 
void signal_handler(int signal)
{
  gSignalStatus = signal;
  T5LOG(T5TRANSACTION) << " Received signal "<< signal<< "; Shutting down :";
  OtmMemoryServiceWorker::getInstance()->shutdownService(signal);
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

//using namespace gflags;
using namespace google;
//using namespace GFLAGS_NAMESPACE;


int proxygen_server_init();
int main(int argc, char* argv[]) {
   setlocale(LC_ALL, "");
   std::signal(SIGINT, signal_handler);
   std::signal(SIGABRT, signal_handler);
   std::signal(SIGKILL, signal_handler);
   //#ifdef GLOGGING_ENABLED
   if(FLAGS_log_dir.empty()){
       FLAGS_log_dir = "/root/.t5memory/LOG/";
   }
   //std::string initLogMsg;
   //if(!FilesystemHelper::DirExists(FLAGS_log_dir)){
   //     initLogMsg += "Log directory created, path = " + FLAGS_log_dir;
   //     FilesystemHelper::CreateDir(FLAGS_log_dir, 0700);
   //}
   FLAGS_colorlogtostderr = true;
   FLAGS_logtostderr = true;
   //FLAGS_localhostonly = true;
   //FLAGS_port = 4045;
   FLAGS_v=2;
   //#endif
   //#ifdef GFLAGS_ENABLED
   FLAGS_alsologtostderr = true;
   FLAGS_t5loglevel = T5DEVELOP;
   //google::InstallFailureSignalHandler();
   // google::InstallFailureWriter(FailureWriter);
   google::ParseCommandLineFlags(&argc, &argv, true);
   //gflags::ParseCommandLineFlags(&argc, &argv, false);
   //#endif//GFLAGS_ENABLED

   //#ifdef GLOGGING_ENABLED
   google::InitGoogleLogging(argv[0]);//, &CustomPrefix); 
   
   T5LOG(T5TRANSACTION) <<"Worker thread starting, v = "<<T5GLOBVERSION<<"."<< T5MAJVERSION<<"."<<T5MINVERSION;   
   std::thread worker(proxygen_server_init);
   worker.join();
   OtmMemoryServiceWorker::getInstance()->shutdownService(SHUTDOWN_CALLED_FROM_MAIN);
   while(OtmMemoryServiceWorker::getInstance()->fServiceIsRunning == true);
   T5LOG(T5TRANSACTION) << "Worker thread finished";    
}

