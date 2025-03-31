#ifndef _LOG_WRAPPER_H_
#define _LOG_WRAPPER_H_

#include <string>
#include <glog/logging.h>
#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
#include <boost/stacktrace.hpp>

#define _DEBUG_LOGS

#ifdef _DEBUG_LOGS
  #define TOKENIZELOG
  #define SPELLFACTORY_LOGGING
  //#define MEASURETIME
  #define SGMLDITA_LOGGING
  #define INLINE_TAG_REPL_LOGGING
  #define _NTGET_LOG
  #define MATCHLEVEL_LOGGING
  #define MATCHLIST_LOG
  #define IMPORT_LOGGING
  #define EQFMEMORYLOGGING
  #define TEMPORARY_COMMENTED_LOGS
  #define MARKUPTABLE_LOGGING
#endif

/*
namespace boost { namespace stacktrace {

template <class CharT, class TraitsT, class Allocator>
std::basic_ostream<CharT, TraitsT>& do_stream_st(std::basic_ostream<CharT, TraitsT>& os, const basic_stacktrace<Allocator>& bt) {
    const std::streamsize w = os.width();
    const std::size_t frames = bt.size();
    for (std::size_t i = 0; i < frames; ++i) {
        os.width(2);
        os << i;
        os.width(w);
        os << "# ";
        os << bt[i].name();
        os << '\n';
    }

    return os;
}
template <class CharT, class TraitsT, class Allocator>
std::basic_ostream<CharT, TraitsT>& do_stream_st(std::basic_ostream<CharT, TraitsT>& os, const basic_stacktrace<Allocator>& bt);

template <class CharT, class TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, const stacktrace& bt) {
    return do_stream_st(os, bt);
}

}}  // namespace boost::stacktrace
//*/

#define GET_NAME(x) #x
#define toStr(i) std::to_string(i)

enum LOGLEVEL{
    T5DEVELOP=0,
    T5DEBUG=1,
    T5INFO=2,
    T5WARNING=3,
    T5ERROR=4,
    T5FATAL=5,
    T5TRANSACTION=6 // not an error, but for separate logs
};

enum LOGCODES{
    LOG_NO_ERROR = 0,
    LOG_SMALLER_THAN_TRESHOLD,
    LOG_FAIL_INIT,
    LOG_FAIL_OPEN_FILE
};


#include <algorithm>

std::string removeNewlines(const std::string& input) ;

class NoNewlineLogSink : public google::LogSink {
public:
    void send(google::LogSeverity severity, const char* full_filename, 
              const char* base_filename, int line, const struct ::tm* tm_time, 
              const char* message, size_t message_len) override ;

    void WaitTillSent() override ;
};


class CustomLogMessage : public google::LogMessage {
public:
    CustomLogMessage(const char* file, int line, google::LogSeverity severity)
        : google::LogMessage(file, line, severity) {} ;

    ~CustomLogMessage();
};


// Macro to use the custom log message class
#define LOG_CUSTOM(severity) \
    CustomLogMessage(__FILE__, __LINE__, google::GLOG_##severity).stream()


class T5Logger{
    T5Logger(){
        fFilterLogs = false;
        logLevelTreshold = T5DEVELOP;
        
    }
public:
    bool fFilterLogs; // during init should be set to false to print init messages
    // then could be set to true to filter(to not print) messages about successfull requests
    int logLevelTreshold;
    int requestType;
    
    google::NullStream nullStream;
    
    static T5Logger* GetInstance();
    static std::ostringstream& __getLogBuffer();
    static std::ostringstream& __getBodyBuffer();
    int LogStop();

    int DesuppressLoggingInFile();
    int suppressLogging();
    int desuppressLogging(int prevTreshold);

    int ResetLogBuffer();
    int SetLogBuffer(std::string logMsg);
    int SetLogInfo(int RequestType);
    int AddToLogBuffer(std::string logMsg);
    int SetBodyBuffer(std::string logMsg);
    std::string FlushLogBuffer();
    std::string FlushBodyBuffer();

    int SetLogLevel(int level);
    int GetLogLevel();
    bool CheckLogLevel(int level);
    void SetLogFilter(bool filterOn);

    std::string FlushBuffers(int severity);
};

void GetElapsedTime( long long *plTime );
std::string getTimeStr();
int getBuffIdForLog(int severity);

#define GET_STACKTRACE_EXPL boost::stacktrace::stacktrace()
//#define GET_STACKTRACE boost::stacktrace::stacktrace()
#define GET_STACKTRACE ""


#define FLUSHLOGBUFFERS T5Logger::GetInstance()->FlushBuffers(T5ERROR)
//#define FLUSHLOGBUFFERS ""


//#define CUSTOM_LOG_MESSAGE
#ifdef CUSTOM_LOG_MESSAGE

    #define _T5_LOG_T5DEVELOP CustomLogMessage(__FILE__, __LINE__, google::INFO).stream()
    #define _T5_LOG_T5DEBUG CustomLogMessage(__FILE__, __LINE__, google::INFO).stream()
    #define _T5_LOG_T5INFO CustomLogMessage(__FILE__, __LINE__, google::INFO).stream()
    #define _T5_LOG_T5TRANSACTION CustomLogMessage(__FILE__, __LINE__, google::INFO).stream()
    #define _T5_LOG_T5WARNING CustomLogMessage(__FILE__, __LINE__, google::WARNING).stream()
    #define _T5_LOG_T5ERROR CustomLogMessage(__FILE__, __LINE__, google::ERROR).stream()
    #define _T5_LOG_T5FATAL CustomLogMessage(__FILE__, __LINE__, google::ERROR).stream()

    #define T5LOG_FROM_ID(id, severity) static_cast<void>(0), \
    id == -1 ? google::LogMessageVoidify() & T5Logger::GetInstance()->nullStream \
        : id == -2 ? google::LogMessageVoidify() & T5Logger::__getLogBuffer() \
            : google::LogMessageVoidify() & _T5_LOG_##severity

#define T5LOG(severity) T5LOG_FROM_ID(getBuffIdForLog(severity), severity) << \
    "::[" << #severity << "]::" << __func__ << ": "

#else
    #define _T5_LOG_T5DEVELOP  LOG(INFO)
    #define _T5_LOG_T5DEBUG  LOG(INFO)
    #define _T5_LOG_T5INFO LOG(INFO)
    #define _T5_LOG_T5TRANSACTION LOG(INFO)
    #define _T5_LOG_T5WARNING LOG(WARNING) << GET_STACKTRACE
    #define _T5_LOG_T5ERROR LOG(ERROR) << GET_STACKTRACE << FLUSHLOGBUFFERS
    #define _T5_LOG_T5FATAL LOG(ERROR) << GET_STACKTRACE << FLUSHLOGBUFFERS

    #define T5LOG_FROM_ID(id, severity)  static_cast<void>(0), \
                id == -1 ? google::LogMessageVoidify() & T5Logger::GetInstance()->nullStream \
                    : id == -2?   google::LogMessageVoidify() &  T5Logger::__getLogBuffer() \
                        : google::LogMessageVoidify() & _T5_LOG_##severity

    #define T5LOG(severity)   T5LOG_FROM_ID(getBuffIdForLog(severity), severity) << \
                "::[" << #severity << "]::" << __func__ << ": " 
#endif //CUSTOM_LOG_MESSAGE

#define LOG_UNIMPLEMENTED_FUNCTION T5LOG(T5FATAL) <<__FILE__ << ":" << __LINE__ << ": called unimplemented function in "  << __func__ << "; stacktrace: " << GET_STACKTRACE_EXPL;

//#define LOG_TEMPORARY_COMMENTED T5LOG(T5INFO) <<__FILE__ << ":" << __LINE__ << ": called temporary commented code in "  << __func__ << "; stacktrace: " << GET_STACKTRACE_EXPL << "; " 
#define LOG_TEMPORARY_COMMENTED_W_INFO(info) if( VLOG_IS_ON(2) && T5Logger::GetInstance()->CheckLogLevel(T5DEBUG)) T5LOG(T5ERROR) <<__FILE__ << ":" << __LINE__ << ": called temporary commented code in "  << __func__ << "; stacktrace: " << GET_STACKTRACE_EXPL << "; " << info ;


#define LOG_ERROR_SETTER_LINES

//#ifdef LOG_ERROR_SETTER_LINES
    #define SET_AND_LOG(rc, error)  rc = error; T5LOG(T5DEBUG) << "rc was set to " << #error  
//#else
//    #define SET_AND_LOG(rc, error) rc = error;
//#endif

#define LOG_AND_SET_RC(rcVar, logLevel, rc)  T5LOG(logLevel) << "rc was set to " <<#rc << "(" << rc <<"), prev state = " <<rcVar; rcVar = rc;
#endif //_LOG_WRAPPER_H_