#ifndef _LOG_WRAPPER_H_
#define _LOG_WRAPPER_H_

#include <string>
#include <glog/logging.h>
#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
#include <boost/stacktrace.hpp>
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

std::string getTimeStr();
int getBuffIdForLog(int severity);

//#define GET_STACKTRACE boost::stacktrace::stacktrace()
#define GET_STACKTRACE ""

#define FLUSHLOGBUFFERS T5Logger::GetInstance()->FlushBuffers(T5ERROR)
//#define FLUSHLOGBUFFERS ""

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

#define LOG_UNIMPLEMENTED_FUNCTION T5LOG(T5FATAL) <<__FILE__ << ":" << __LINE__ << ": called unimplemented function in "  << __func__;

// code after this line should be deleted and replaced in calls with T5LOG
int LogMessage(int LogLevel, std::string message1);
int LogMessage(int LogLevel, std::string message1, std::string message2);
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3);
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4);
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5);
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6);
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7);
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8);

int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8,
                    std::string message9);
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8,
                    std::string message9, std::string message10) ;
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8,
                    std::string message9, std::string message10, std::string message11) ;

#endif //_LOG_WRAPPER_H_