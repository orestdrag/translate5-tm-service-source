
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
#include <sys/personality.h>


#if defined(__has_feature)
#  if __has_feature(address_sanitizer)
#    define ADDRESS_SANITIZER_ENABLED 1
#  endif
#endif

#if !defined(ADDRESS_SANITIZER_ENABLED) && defined(__SANITIZE_ADDRESS__)
#  define ADDRESS_SANITIZER_ENABLED 1
#endif




static bool ValidatePort(const char* flagname, int32_t value) {
   if (value > 0 && value < 32768)   // value is ok
     return true;
   printf("Invalid value for --%s: %d\n", flagname, (int)value);
   return false;
}



DEFINE_int64(tmMaxIdleTimeSec, 0, "If set some value, except, 0, during request, tm list would bee cleaned up from tms that are longer inactive than this number");


DEFINE_int32(port, 4080, "What port to listen on");
DEFINE_validator(port, &ValidatePort);

DEFINE_string(t5_ip, "", "Which ip to use in t5memory(default is any). Should be in format '1.1.1.1', default is to listen to all available ip");


DEFINE_string(servicename, "t5memory", "Sets service name to use in url");

DEFINE_bool(limit_num_of_active_requests, false, 
    "If set to true, it would be possible to handle only up to servicethreads-1 requests at the same time, the last thread would respond with 503 to eliminate creating queue of requests waiting to be handled.");

static bool ValidateTriplesThreshold(const char* flagname, int32_t value) {
   if (value >= 0 && value <= 100)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [0...100]\n", flagname, (int)value);
   return false;
}

DEFINE_int32(triplesthreshold, 5, "Sets threshold to pre fuzzy filtering based on hashes of neibour tokens");
DEFINE_validator(triplesthreshold, &ValidateTriplesThreshold);

DEFINE_bool(log_tm_lifetime, false, " If set to true, and --v=2 and --t5loglevel=4, TMs ctor and dctor would have transaction level logs");

DEFINE_bool(log_hashes_in_hash_sentence, false, " If set to true, and --v=2 and --t5loglevel=4, tokens and their hashed would be logged");
DEFINE_bool(add_tokens_to_fuzzy, false, " If set to true, list of tokens would be returned in fuzzy responce. Could make execution a bit slower");

DEFINE_bool(add_premade_socket, false, "if set to true, socket instance would be created outside of proxygen and then binded, that made possible to add tcp backog event handler and use socket_backog option");
DEFINE_bool(log_tcp_backog_events, false, "if set to true, tcp backlog events would be logged(to enable, add_premade_socket flag should be set to true)");

static bool ValidateHttpListenBacklog(const char* flagname, int64_t value) {
   if (value >= 0 && value <= 4096)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [0...4096]\n", flagname, (int)value);
   return false;
}

DEFINE_int64(http_listen_backlog, 128/*1024*/, "Sets http options listen backog");
DEFINE_validator(http_listen_backlog, &ValidateHttpListenBacklog);

DEFINE_int64(socket_backlog, 1024, "Sets proxygen socket listen backog(disabled, to enable set add_premade_socket=true)");
DEFINE_validator(socket_backlog, &ValidateHttpListenBacklog);

static bool ValidateTimeout(const char* flagname, int32_t value) {
   if (value >= 0 && value <= 3600000)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [0...3600000]\n", flagname, (int)value);
   return false;
}
DEFINE_int32(timeout, 180000, "Sets timeout for service request handling");
DEFINE_validator(timeout, &ValidateTimeout);


DEFINE_bool(flush_tm_at_shutdown, false, "If set to true, flushes tm when shutting down the app not using shutdown request");
DEFINE_bool(wait_for_import_and_reorganize_requests, true, "If set to true, waiting for all import and reorganize processes to be done at shutdown when not using shutdown request");


DEFINE_bool(log_every_request_start, false, "Sets log for every request call with it's url, method etc...");
DEFINE_bool(log_every_request_end  , false, "Sets log for every request end  with it's url, method etc...");

DEFINE_bool(flush_tm_to_disk_with_every_update, false, "If set to true, flushes tm to disk with every successfull update request");


static bool ValidateThreads(const char* flagname, int32_t value) {
    if(FLAGS_limit_num_of_active_requests && (value < 2 || value > 100)){
      printf("Invalid value for --%s: %d, should be [2...100], because limit_num_of_active_requests is set to true\n", flagname, (int)value);
      return false;
    }
    if (value < 1 || value > 100){ 
      printf("Invalid value for --%s: %d, should be [1...100]\n", flagname, (int)value);
      return false;
    }
    return true;
}

DEFINE_int32(servicethreads, 5, "Sets amought of worker threads for service");
DEFINE_validator(servicethreads, &ValidateThreads);


static bool ValidateRAM(const char* flagname, int64_t value) {
   if (value >= 1 && value <= 100000)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [1...100000]\n", flagname, (int)value);
   return false;
}

DEFINE_int64(allowedram, 5000, "Sets amought RAM(in MB) allowed for service to use");
DEFINE_validator(allowedram, &ValidateRAM);


static bool ValidateTMDSize(const char* flagname, int64_t value) {
   if (value >= 1 && value <= 10000)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [1...10000]\n", flagname, (int)value);
   return false;
}

DEFINE_int64(allowedtmdsize, 190, "Sets max size of tmd file(in MB) after which t5m would not allow to add new data to the tm");
DEFINE_validator(allowedtmdsize, &ValidateTMDSize);

static bool ValidateLOGlevel(const char* flagname, int32_t value) {
   if (value >= 0 && value <= 6)   // value is ok
     return true;
   printf("Invalid value for --%s: %d, should be [0...6]\n", flagname, (int)value);
   return false;
}

DEFINE_int32(t5loglevel, T5INFO, "Sets t5memory log level threshold from DEVELOP(0) to TRANSACTION(6)");
DEFINE_validator(t5loglevel, &ValidateLOGlevel);

DEFINE_int64(tmRequestLockDefaultTimeout, 3000, 
  "Sets tm mutex lock timeout(in ms) for part where request is requesting tm(which is used to open and close tms, and hold list of opened tms), after which operation would be canceled and mutex would return an error, if set to 0, mutex lock would be waited without timeout");

DEFINE_int64(tmLockDefaultTimeout, 3000, 
  "Sets tm mutex lock timeout(in ms) for TM after which operation would be canceled and mutex would return an error, if set to 0, mutex lock would be waited without timeout");

DEFINE_int64(tmListLockDefaultTimeout, 3000,
   "Sets tm mutex lock timeout(in ms) for TM list(which is used to open and close tms, and hold list of opened tms), after which operation would be canceled and mutex would return an error, if set to 0, mutex lock would be waited without timeout");

DEFINE_int64(debug_sleep_in_request_run, 0,
   "If set, provide artificial delay in every request handling execution equal to provided num of microseconds");

DEFINE_bool(disable_aslr, false, "If set to true, process personality would be set to ADDR_NO_RANDOMIZE");

DEFINE_bool(useTimedMutexesForReorganizeAndImport, false, 
  "If set to true, in reorganize or import thread would be used mutexes with timeouts, and reorganizee or import could be canceled, false(by default) - would be used non timed mutexes");

DEFINE_bool(allowLoadingMultipleTmsSimultaneously, false, 
  "If set to true, multiple tms could be loaded from the disk at the same time. ");


DEFINE_bool(keep_tm_backups, true, "if set to true, when saving tmd and tmi files, old copies would be saved with .old suffix");


DEFINE_bool(logMutexes, false, "if set to true you would see mutex logs");

DEFINE_bool(useconfigfile, false, "Set to use values from config file that should be located under ~/.t5memory/t5memory.conf");


DEFINE_bool(forbiddeletefiles, false, "Set to true to keep all files(including temporary and tm)");

DEFINE_bool(ignore_newer_target_exists_check, true, "if set to true, check for newer already saved target would be skipped for saving segments" );


DEFINE_bool(log_memmove_in_compareputdata, false, "if set to true, when saving segment and causing memmove in compareputdata functions, just before memmove, data would be logged - use this to debug btree crashes." );

DEFINE_bool(enable_newlines_in_logs, false, 
        "(not working)if set to true, would keep newline symbols in the logs, otherwise(by default) newlines would be removed and logs would be oneliners" );


void handle_interrupt_sig(int sig) {
    T5LOG( T5TRANSACTION) << "Received interrupt signal";
    //StopOtmMemoryService();
    T5LOG( T5TRANSACTION) << "Stopping t5memory";

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
    T5LOG( T5FATAL) << ":backtrace:" << backtraceStr ;

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
  T5LOG(T5TRANSACTION) << " Received signal "<< signal<< "; Saving TM's and Shutting down :";
  MutexTimeout mt{0};

  ShutdownRequestData::CheckImportFlushTmsAndShutdown(signal, mt, FLAGS_flush_tm_at_shutdown, FLAGS_wait_for_import_and_reorganize_requests);

  //ShutdownRequestData srd;
  //srd.sig = SHUTDOWN_CALLED_FROM_MAIN;
  //srd.sig = signal;
  //srd.run();
  //while( TMManager::GetInstance()->fServiceIsRunning == true);
  T5LOG(T5TRANSACTION) <<"signal handler finished work";
  sleep(1);
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

#include <iomanip>
std::string to_hex(unsigned long src){
  // Create a stringstream object
  std::stringstream stream;

  // Convert the decimal value to hexadecimal and store it in the stringstream
  stream << "0x"
         << std::setfill('0') << std::setw(sizeof(unsigned long) * 2)
         << std::hex << src;

  // Retrieve the hexadecimal string from the stringstream
  std::string hex_value = stream.str();
  return hex_value;
}

int proxygen_server_init();
int main(int argc, char* argv[]) {
  //FLAGS_disable_aslr = FLAGS_add_premade_socket = FLAGS_log_tcp_backog_events = true;
  bool fAsanEnabled = false;

#if defined(ADDRESS_SANITIZER_ENABLED)
// Code specific to AddressSanitizer
  fAsanEnabled = true;
#endif

  if(FLAGS_disable_aslr){
    personality(ADDR_NO_RANDOMIZE);
    unsigned long pers = personality(0xffffffff);
    // Check if ASLR is enabled (ADDR_NO_RANDOMIZE is 0x0040000)
    if (!(pers & ADDR_NO_RANDOMIZE)) {
        // Disable ASLR by setting ADDR_NO_RANDOMIZE flag
        if (personality(pers | ADDR_NO_RANDOMIZE) == -1) {
            perror("personality");
            return 1;
        }
    }
  }   
   setlocale(LC_ALL, "");
   std::signal(SIGINT, signal_handler);
   std::signal(SIGABRT, signal_handler);
   std::signal(SIGKILL, signal_handler);
   std::signal(SIGTERM, signal_handler);
   
   //#ifdef GLOGGING_ENABLED
   if(FLAGS_log_dir.empty()){
       FLAGS_log_dir = "/root/.t5memory/LOG/";
   }

  //FLAGS_log_dir = "~/t5memory/LOG";
   //std::string initLogMsg;
   //if(!FilesystemHelper::DirExists(FLAGS_log_dir)){
   //     initLogMsg += "Log directory created, path = " + FLAGS_log_dir;
   //     FilesystemHelper::CreateDir(FLAGS_log_dir, 0700);
   //}
   FLAGS_colorlogtostderr = true;
   //FLAGS_ignore_newer_target_exists_check = true;
   //FLAGS_logtostderr = true;
   //FLAGS_localhostonly = true;
   //FLAGS_port = 4045;
   //FLAGS_v=2;
   //#endif
   //#ifdef GFLAGS_ENABLED
   FLAGS_alsologtostderr = true;
   //FLAGS_t5loglevel = 0;
   //FLAGS_log_every_request_end = FLAGS_log_every_request_start = true;
   //FLAGS_http_listen_backlog = FLAGS_socket_backlog = 32;//128;
   //FLAGS_debug_sleep_in_request_run = 10000000;

   //google::InstallFailureSignalHandler();
   // google::InstallFailureWriter(FailureWriter);
   google::ParseCommandLineFlags(&argc, &argv, true);
   //gflags::ParseCommandLineFlags(&argc, &argv, false);
   //#endif//GFLAGS_ENABLED

   //#ifdef GLOGGING_ENABLED
   google::InitGoogleLogging(argv[0]);//, &CustomPrefix); 
   google::InstallFailureSignalHandler();
   // Install the custom LogSink
   //static NoNewlineLogSink sink;
   //google::AddLogSink(&sink);
   T5LOG(T5TRANSACTION) <<"Worker thread starting, v = "<<T5GLOBVERSION<<"."<< T5MAJVERSION<<"."<<T5MINVERSION << "; asan = " << fAsanEnabled; 
   
  // Get the current personality flags
  unsigned long pers = personality(0xffffffff);
  // Continue with the rest of your program
  T5LOG(T5TRANSACTION) << "personality now: " << to_hex(pers);

   std::thread worker(proxygen_server_init);
   worker.join();
   //TODO::REFACTOR HERE
   
   //signal_handler(SHUTDOWN_CALLED_FROM_MAIN);
   signal_handler(SIGINT);
   T5LOG(T5TRANSACTION) << "Worker thread finished";    
}

