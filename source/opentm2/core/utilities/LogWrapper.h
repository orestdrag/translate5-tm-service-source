#ifndef _LOG_WRAPPER_H_
#define _LOG_WRAPPER_H_

#include <string>
#include <glog/logging.h>

//#include "FilesystemHelper.h"

#define toStr(i) std::to_string(i)




enum LOGLEVEL{
    T5DEVELOP=0,
    //DEVELOP = 0,
    T5DEBUG=1,
    //DEBUG = 1,
    T5INFO=2,
    //INFO = 2,
    T5WARNING=3,
    //WARNING = 3,
    T5ERROR=4,
    //ERROR = 4,
    T5FATAL=5,
    T5TRANSACTION=6 // not an error, but for separate logs
    //TRANSACTION = 6
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
        __googleBuff = new google::LogMessage(__FILE__,__LINE__);
    }
public:
    google::LogMessage* __googleBuff;
    bool fFilterLogs; // during init should be set to false to print init messages
    // then could be set to true to filter(to not print) messages about successfull requests
    int logLevelTreshold;
    int requestType;
    
    google::NullStream nullStream;
    
    static T5Logger* GetInstance();
    
    //static std::ostringstream& __getLogBuffer();
    static std::ostream& __getLogBuffer();
    //static std::ostringstream& getLogBuffer();//use this to save data in buff with mark {lb}

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







/*

std::string LogMessage;

    LogMessage += message;
    if(T5Logger::GetInstance()->CheckLogLevel(T5DEBUG) == FALSE && LogMessage.size()> 4000){
        LogMessage.resize(3000);
        LogMessage += ". . . (truncated) ";
    }
*/
/*
#define COMPACT_T5_LOG_T5DEVELOP LOG(INFO)     << "[DEVELOP]::"
#define COMPACT_T5_LOG_T5DEBUG LOG(INFO)       << "[DEBUG]::"
#define COMPACT_T5_LOG_T5INFO LOG(INFO)        << "[INFO]::"
#define COMPACT_T5_LOG_T5TRANSACTION LOG(INFO) << "[TRANSACTION]::"
#define COMPACT_T5_LOG_T5WARNING LOG(WARNING)  << "[WARNING]::"
#define COMPACT_T5_LOG_T5ERROR LOG(ERROR)      << "[ERROR]::"
#define COMPACT_T5_LOG_T5FATAL LOG(DFATAL)     << "[FATAL]::"
//*/
#define _T5_DIR_LOG_T5DEVELOP   LOG(INFO)
#define _T5_DIR_LOG_T5DEBUG  LOG(INFO)
#define _T5_DIR_LOG_T5INFO LOG(INFO)
#define _T5_DIR_LOG_T5TRANSACTION LOG(INFO)
#define _T5_DIR_LOG_T5WARNING LOG(WARNING)
#define _T5_DIR_LOG_T5ERROR LOG(ERROR)
#define _T5_DIR_LOG_T5FATAL LOG(DFATAL)

//#define _T5_BUF_LOG_T5DEVELOP  static_cast<void>(0), VLOG_IS_ON(2)  ? (void) 0 : google::LogMessageVoidify() & LOG(INFO)
//#define _T5_BUF_LOG_T5DEBUG LOG(INFO) VLOG_IS_ON(1)  ? (void) 0 : google::LogMessageVoidify() & LOG(INFO)
//#define _T5_BUF_LOG_T5INFO LOG(INFO) VLOG_IS_ON(1)  ? (void) 0 : google::LogMessageVoidify() & LOG(INFO)
/*
#define _T5_BUF_LOG_T5DEVELOP static_cast<void>(0), T5Logger::GetInstance()->logLevelTreshold > T5DEVELOP || !VLOG_IS_ON(1) ? \
    (void) 0  : google::LogMessageVoidify() &  T5Logger::GetInstance()->__getLogBuffer() << "BUFFERED"
#define _T5_BUF_LOG_T5DEBUG LOG(INFO) (T5Logger::GetInstance()->logLevelTreshold > T5DEBUG || !VLOG_IS_ON(1)) ? \
    (void) 0  : google::LogMessageVoidify() &  T5Logger::GetInstance()->__getLogBuffer() << "BUFFERED"
#define _T5_BUF_LOG_T5INFO  T5Logger::GetInstance()->logLevelTreshold > T5INFO || !VLOG_IS_ON(1) ? \
    (void) 0 : google::LogMessageVoidify() &  T5Logger::GetInstance()->__getLogBuffer() << "BUFFERED"
#define _T5_BUF_LOG_T5TRANSACTION google::LogMessageVoidify() & LOG(INFO) 
#define _T5_BUF_LOG_T5WARNING T5Logger::GetInstance()->logLevelTreshold > T5WARNING || !VLOG_IS_ON(1) ? \
    (void) 0 : google::LogMessageVoidify() &  T5Logger::GetInstance()->__getLogBuffer() << "BUFFERED"
#define _T5_BUF_LOG_T5ERROR  google::LogMessageVoidify() & LOG(ERROR) << T5Logger::GetInstance()->FlushLogBuffer() << "\n" << T5Logger::GetInstance()->FlushBodyBuffer() 
#define _T5_BUF_LOG_T5FATAL google::LogMessageVoidify() &  LOG(DFATAL) << T5Logger::GetInstance()->FlushLogBuffer() << "\n" << T5Logger::GetInstance()->FlushBodyBuffer() 
//*/

//#define _T5_LOG_T5DEVELOP static_cast<void>(0), T5Logger::GetInstance()->logLevelTreshold > T5DEVELOP || !VLOG_IS_ON(1) ? \
         (void) 0  :  VLOG_IS_ON(2) ?  google::LogMessageVoidify() & LOG(INFO) \
          :          !VLOG_IS_ON(1)?  (void) 0: T5Logger::GetInstance()->__getLogBuffer()
//#define _T5_LOG_T5DEVELOP ( static_cast<void>(0), T5Logger::GetInstance()->logLevelTreshold > T5DEVELOP || !VLOG_IS_ON(1) ? \
         (void) 0  :  VLOG_IS_ON(2) ?  google::LogMessageVoidify() & LOG(INFO) : T5Logger::GetInstance()->__getLogBuffer() )
#define _T5_LOG_T5DEVELOP  LOG(INFO)
#define _T5_LOG_T5DEBUG  LOG(INFO)
#define _T5_LOG_T5INFO LOG(INFO)
#define _T5_LOG_T5TRANSACTION LOG(INFO)
#define _T5_LOG_T5WARNING LOG(WARNING)
#define _T5_LOG_T5ERROR LOG(ERROR)
#define _T5_LOG_T5FATAL LOG(ERROR)

#define _T5_BUFF_LOG_T5DEVELOP  static_cast<void>(0), \
         !(VLOG_IS_ON(1))? \
          (void) 0  :   google::LogMessageVoidify() & T5Logger::GetInstance()->__getLogBuffer()
#define _T5_BUFF_LOG_T5DEBUG  LOG(INFO)
#define _T5_BUFF_LOG_T5INFO LOG(INFO)
#define _T5_BUFF_LOG_T5TRANSACTION LOG(INFO)
#define _T5_BUFF_LOG_T5WARNING LOG(WARNING)
#define _T5_BUFF_LOG_T5ERROR LOG(ERROR)
#define _T5_BUFF_LOG_T5FATAL LOG(DFATAL)

//#define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
         (T5Logger::GetInstance()->logLevelTreshold > severity)? \
          (void) 0  :  static_cast<void>(0), \
            (!T5Logger::GetInstance()->fFilterLogs || VLOG_IS_ON(2))?  \
                  google::LogMessageVoidify() & _T5_LOG_##severity : static_cast<void>(0), \
                  (severity < T5ERROR)? \
                    (void) T5Logger::GetInstance()->__getLogBuffer() : google::LogMessageVoidify() & (_T5_LOG_##severity << T5Logger::GetInstance()->FlushLogBuffer() << "\n" << T5Logger::GetInstance()->FlushBodyBuffer())

//#define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
         (T5Logger::GetInstance()->logLevelTreshold > severity)? \
           google::LogMessageVoidify() & T5Logger::GetInstance()->nullStream  :  static_cast<void>(0), \
            (!T5Logger::GetInstance()->GetInstance()->fFilterLogs || VLOG_IS_ON(2) || severity>=T5ERROR )?  \
                  google::LogMessageVoidify() & _T5_LOG_##severity : static_cast<void>(0), \
                   (!VLOG_IS_ON(1))? \
                   google::LogMessageVoidify() & T5Logger::GetInstance()->nullStream  :  google::LogMessageVoidify() & T5Logger::GetInstance()->__getLogBuffer()

int getBuffIdForLog(int severity);
std::ostream& getBuffForLog(int severity);

#define _T5LOG_BUFF_DIR_CHECK(severity)  \
         (T5Logger::GetInstance()->logLevelTreshold > severity)? \
           google::LogMessageVoidify() & T5Logger::GetInstance()->nullStream  :  static_cast<void>(0), \
            (!T5Logger::GetInstance()->GetInstance()->fFilterLogs || VLOG_IS_ON(2) || severity>=T5ERROR )?  \
                  google::LogMessageVoidify() & _T5_LOG_##severity : static_cast<void>(0), \
                   (!VLOG_IS_ON(1))? \
                   google::LogMessageVoidify() & T5Logger::GetInstance()->nullStream  :  google::LogMessageVoidify() & T5Logger::GetInstance()->__getLogBuffer()


//#define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
         (T5Logger::GetInstance()->logLevelTreshold > severity)? \
          (void) T5Logger::GetInstance()->nullStream  :  static_cast<void>(0), \
            (!T5Logger::GetInstance()->GetInstance()->fFilterLogs || VLOG_IS_ON(2) || severity>=T5ERROR )?  \
                  google::LogMessageVoidify() & _T5_LOG_##severity : static_cast<void>(0), \
                   (!VLOG_IS_ON(1))? \
                   (void) T5Logger::GetInstance()->nullStream  : (void) T5Logger::GetInstance()->__getLogBuffer()

//#define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
         (T5Logger::GetInstance()->logLevelTreshold > severity)? \
          (void) 0  :  static_cast<void>(0), \
            (!T5Logger::GetInstance()->fFilterLogs || VLOG_IS_ON(2) || severity>=T5ERROR )?  \
                  google::LogMessageVoidify() & _T5_LOG_##severity : static_cast<void>(0), \
                   (!VLOG_IS_ON(1))? \
                   (void) 0 : (void) T5Logger::GetInstance()->__getLogBuffer()
                      ///good but some logs still empty                
//#define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
//         (T5Logger::GetInstance()->logLevelTreshold > severity)? \
//          (void) 0  :  static_cast<void>(0), \
//            (!T5Logger::GetInstance()->fFilterLogs || VLOG_IS_ON(2) || severity>=T5ERROR )?  \
//                  google::LogMessageVoidify() & _T5_LOG_##severity : static_cast<void>(0), \
//                  (severity < T5ERROR)? \
//                    (void) T5Logger::GetInstance()->__getLogBuffer() : google::LogMessageVoidify() & _T5_LOG_##severity 
                    


//#define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
         (T5Logger::GetInstance()->logLevelTreshold > severity)? \
          (void) 0  :  static_cast<void>(0), \
            (T5Logger::GetInstance()->fFilterLogs && !VLOG_IS_ON(2))?  \
               google::LogMessageVoidify() &  _T5_BUFF_LOG_##severity : google::LogMessageVoidify() & _T5_LOG_##severity

//#define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
         T5Logger::GetInstance()->fFilterLogs && !VLOG_IS_ON(2) ? \
          _T5_BUF_LOG_##severity  : google::LogMessageVoidify() & _T5_DIR_LOG_##severity
// #define _T5LOG_BUFF_DIR_CHECK(severity) static_cast<void>(0), \
//         T5Logger::GetInstance()->fFilterLogs && !T5Logger::GetInstance()->CheckLogLevel(severity) ? \
//          _T5_BUF_LOG_##severity  : google::LogMessageVoidify() & _T5_DIR_LOG_##severity        
         //(void) 0 : google::LogMessageVoidify() & _T5_DIR_LOG_##severity


//#define T5LOG(severity)   T5LOG_BUFF_DIR_CHECK(severity) << \
            "::[" << #severity << "]::" << __func__ << ": " << (severity >= T5ERROR ? T5Logger::GetInstance()->FlushLogBuffer() + T5Logger::GetInstance()->FlushBodyBuffer() : "" )

//#define T5LOG(severity)   getBuffForLog(severity) << \
//            "::[" << #severity << "]::" << __func__ << ": " << (severity >= T5ERROR ? T5Logger::GetInstance()->FlushLogBuffer() + T5Logger::GetInstance()->FlushBodyBuffer() : "" )

#define T5LOG_FROM_ID(id, severity)  static_cast<void>(0), \
            id == -1 ? google::LogMessageVoidify() & T5Logger::GetInstance()->nullStream \
                : id == -2?   google::LogMessageVoidify() &  T5Logger::GetInstance()->__googleBuff->stream() \
                    : google::LogMessageVoidify() & _T5_LOG_##severity

#define T5LOG(severity)   T5LOG_FROM_ID(getBuffIdForLog(severity), severity) << \
            "::[" << #severity << "]::" << __func__ << ": " << T5Logger::GetInstance()->FlushBuffers(severity) 


//#define T5LOG(severity) static_cast<void>(0), !T5Logger::GetInstance()->CheckLogLevel(severity) ? (void) 0 : google::LogMessageVoidify() & COMPACT_T5_LOG_##severity  << "::[" << #severity << "]::" << __func__ << ": "
//#define T5LOG(severity) //if(T5Logger::GetInstance()->CheckLogLevel(#severity)) \
                        //google::NullStream()     \
                        //else \
 COMPACT_T5_LOG_##severity << __func__


/*
if(VLOG(2) || !fFilterLogs) 
print logs
 if(!VLOG_IS_ON(2) && fFilterLogs){
            if(logLevel < ERROR){
                if( VLOG_IS_ON(1) ){
                    AddToLogBuffer(message + "\n");
                }
            }else{
                if(logLevel == TRANSACTION){
                    LOG(INFO) << message;
                }
                else{                
                    message = FlushLogBuffer() + "\n" 
                            + message + "\n" + FlushBodyBuffer();
                    LOG(ERROR) << message;
                }
            }
        }
//*/

#define LOG_UNIMPLEMENTED_FUNCTION T5LOG(T5FATAL) <<__FILE__ << ":" << __LINE__ << ": called unimplemented function in "  << __func__;


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