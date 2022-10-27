#include "LogWrapper.h"
#include <istream>
#include <sstream>
#include <streambuf>
#include <string>
#include <fstream>
#include <ctime>
#include "FilesystemHelper.h"
#include <iostream>
#include "ThreadingWrapper.h"
#include <mutex>
#include <thread>
#include <cstring>


std::string logFilename;
int logLevelTreshold = DEBUG;
//bool fileLoggingSuppressed = false;
bool fileLoggingSuppressed = true;

bool fFilterLogs = false; // during init should be set to false to print init messages
// then could be set to true to filter(to not print) messages about successfull requests

#define THREAD_ID_WRITE_TO_LOGS 1
#define CONSOLE_LOGGING 1
#define LOGERRORSINOTHERFILE 1
//#define SIMPLE_FILE_LOGGING_ENABLED 1

std::string getDateStr(){
    // current date/time based on current system
   time_t now = time(0);
   
   // convert now to string form
   std::string sDate = ctime(&now);
   int nLinePos = sDate.find('\n');
    if(nLinePos != std::string::npos)
        sDate[nLinePos] = ' ';
   return sDate;
}

std::string getTimeStr(){
    // current date/time based on current system
    time_t now = time(0);
   
    // convert now to string form
    std::string sTime =  ctime(&now);
    int nLinePos = sTime.find('\n');
    if(nLinePos != std::string::npos)
        sTime[nLinePos] = ' ';
    return sTime;
}

std::string generateLogFileName(){
    // current date/time based on current system
   time_t now = time(0);
   
   // convert now to string form
   std::string fName = "Log_" + getDateStr() + ".log";
   return fName;
}



std::mutex logMutex;   

static std::ofstream& getLogFile() {
    static std::ofstream logF;
    if(!logF.is_open()){
        logF.open(logFilename, std::ios_base::app); // append instead of overwrite
    }
    return logF;
}

static std::ofstream& getLogErrorFile() {
    static std::ofstream logErrorF;
    if(!logErrorF.is_open()){
        logErrorF.open(logFilename+"_IMPORTANT", std::ios_base::app); // append instead of overwrite
    }
    return logErrorF;
}

static std::stringstream& getLogBuffer(){
    static std::stringstream logBuff;
    return logBuff;

}


int writeLog(std::string& message, int logLevel){
    #ifdef SIMPLE_FILE_LOGGING_ENABLED
    #ifdef THREAD_ID_WRITE_TO_LOGS
        message += " [thread id = " + std::to_string(_getpid());
        message += "]";
    #endif

    //write log to file
    {
        if(fileLoggingSuppressed){
            message += ", (file logging suppressed)";
        }
        //message += '\n';
        std::lock_guard<std::mutex> logGuard(logMutex); 

        #ifdef CONSOLE_LOGGING
           std::cout << message;
        #endif //CONSOLE_LOGGING

        if( fileLoggingSuppressed == false){
            #ifdef LOGERRORSINOTHERFILE
                if(logLevel >= ERROR  // ONLY IMPORTANT ISSUES SHOULD BE LOGGED HERE
                        && logLevelTreshold < ERROR){ // AND ONLY IF REGULAR LOG THRESHOLD IS LESS THAN ERROR, TO AVOID DUBLICATES
                    getLogErrorFile() << message;
                    //logErrorF << message ; 
                    //logErrorF.flush();
                    //logErrorF.close();
                }
            #endif//LOGERRORSINOTHERFILE
 
            getLogFile() << message;
            if(logLevelTreshold < INFO){
                getLogFile().flush();
                getLogErrorFile().flush();
            }
            //logF.flush();
            //logF.close();
        }
    }
    #endif //SIMPLE_FILE_LOGGING_ENABLED
    
    //#ifdef GLOGGING_ENABLED
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
                    message = FlushLogBuffer() //+ "\n" 
                            + message;
                    LOG(ERROR) << message;
                }
            }
        }else{
            switch(logLevel){
                case DEVELOP:{
                    LOG(INFO) << message;
                    //VLOG(2) << message;                    
                    break;
                }
                case DEBUG:{
                    LOG(INFO) << message;
                    //VLOG(1) << message;
                    break;
                }
                case INFO:{
                    LOG(INFO) << message;
                    break;
                }
                case WARNING:{
                    LOG(WARNING) << message;
                    break;
                }
                case ERROR:{
                    LOG(ERROR) << message;
                    break;
                }
                case FATAL:{
                    //LOG(FATAL) << message;
                    LOG(DFATAL) << message;
                    break;
                }
                case TRANSACTION:{
                    LOG(INFO) << message;
                    break;
                }
            }
        }
    //#endif//GLOG_ENABLED

    return 0;
}
int suppressLoggingInFile(){
    fileLoggingSuppressed = true;
    return 0;
}

int desuppressLoggingInFile(){
    fileLoggingSuppressed = false;
    return 0;
}

int suppressLogging(){
    int prevTreshold = logLevelTreshold;
    logLevelTreshold = FATAL + 1;
    return prevTreshold;
}

int desuppressLogging(int prevTreshold){
    return logLevelTreshold = prevTreshold;
}

int ResetLogBuffer(){
    getLogBuffer().clear();
}

int SetLogBuffer(std::string logMsg){
    ResetLogBuffer();
    getLogBuffer() << logMsg << std::endl;
}

int AddToLogBuffer(std::string logMsg){
    //LOG(INFO) << logMsg;
    //LOG_DEBUG_MSG << logMsg;
    getLogBuffer() << logMsg;// << std::endl;
}

std::string FlushLogBuffer(){
    auto str = getLogBuffer().str();
    ResetLogBuffer();
    return str;
}

int initLog(){
    int prevState = suppressLogging();
    suppressLoggingInFile();

    std::string initLogMsg;
    //TODO: manage files clean-up if there more file
    logFilename = FilesystemHelper::GetOtmDir();
    logFilename += "/LOG";
    if(!FilesystemHelper::DirExists(logFilename)){
        initLogMsg += "Log directory created\n";
        FilesystemHelper::CreateDir(logFilename, 0700);
    }

    logFilename += "/" + generateLogFileName();
    
    initLogMsg += "Log file created, name: ";
    initLogMsg += logFilename + ", time: " + getTimeStr();
    
    desuppressLogging(prevState);
    desuppressLoggingInFile();

    return writeLog(initLogMsg, TRANSACTION);
}


int LogMessageStr(int LogLevel, const std::string& message){
    if(     LogLevel < logLevelTreshold
        || (LogLevel <= DEBUG &&   !V_IS_ON(1))
        || (LogLevel == DEVELOP && !V_IS_ON(2))
        ) {
        return LOG_SMALLER_THAN_TRESHOLD;
    }


    //#ifdef SIMPLE_FILE_LOGGING_ENABLED
    if(logFilename.empty()){
        if(fileLoggingSuppressed == false){
            if( initLog() != 0){
                return LOG_FAIL_INIT;
            }
        }
    }
    //#endif //SIMPLE_FILE_LOGGING_ENABLED

    std::string logMessage;
    switch (LogLevel)
    {
    case FATAL:
        logMessage = "[FATAL]  :";
        break;
    case ERROR:
        logMessage = "[ERROR]  :";
        break;
    case WARNING:
        logMessage = "[WARNING]:";
        break;
    case DEBUG:
        logMessage = "[DEBUG]  :";
        break;
    case INFO:
        logMessage = "[INFO]   :";
        break;
    case DEVELOP:
        logMessage = "[DEVELOP]:";
        break;
    case TRANSACTION:
        logMessage = "[TRANSACTION] ";
        break;
    default:
        break;
    }    

    #ifdef SIMPLE_FILE_LOGGING_ENABLED
    logMessage += " [" + getTimeStr() + "] :: "; 
    #endif //SIMPLE_FILE_LOGGING_ENABLED

    logMessage += message;
    if(CheckLogLevel(DEBUG) == FALSE && logMessage.size()> 4000){
        logMessage.resize(1000);
        logMessage += ". . . ";
    }

    return writeLog(logMessage, LogLevel);
}

void SetLogFilter(bool filterOn){
    fFilterLogs = filterOn;
}

int LogMessage1(int LogLevel, std::string&& message){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message));
}

int LogMessage2(int LogLevel, std::string&& message1, std::string&& message2){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2));
}

int LogMessage3(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3));
}

int LogMessage4(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4));
}
int LogMessage5(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4) + (message5));
}

int LogMessage6(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4) + (message5) + (message6));
}

int LogMessage7(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4) + (message5) + (message6) + (message7));
}

int LogMessage8(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4) + (message5) + (message6) + (message7) + (message8));
}

int LogMessage9(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8,
                        std::string&& message9){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4) + (message5) + (message6) + (message7) + (message8)
                            + (message9));
}

int LogMessage10(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8,
                        std::string&& message9, std::string&& message10){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4) + (message5) + (message6) + (message7) + (message8)
                            + (message9) + (message10));
}


int LogMessage11(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8,
                        std::string&& message9, std::string&& message10, std::string&& message11){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, (message1) + (message2) + (message3)
                            + (message4) + (message5) + (message6) + (message7) + (message8)
                            + (message9) + (message10) + (message11));
}


int LogStop(){
    LogMessage2(TRANSACTION, __func__, " called LogStop -> closing log files");
    {
        std::lock_guard<std::mutex> logGuard(logMutex); 
        if(getLogFile().is_open())
            getLogFile().close();
        if(getLogErrorFile().is_open())
            getLogErrorFile().close();
    }    
    return 0;
}

int DesuppressLoggingInFile(){
    //desuppressLoggingInFile();
    initLog();
    return 0;
}

int SetLogLevel(int level){
    LogMessage2(DEBUG, __func__,":: Setting internal log level was disabled, use commandline\
     flags for verbose level \'--v=[0..2]\' or \'--minloglevel=[0..3]\' instead");
    logLevelTreshold = DEVELOP;
    /*
    if(level <= FATAL && level>=DEVELOP){
        LogMessage2(INFO, "SetLogLevel::Log level treshold was set to ",toStr(level).c_str());
        logLevelTreshold = level;
        return level;
    }else{
        LogMessage3(ERROR,"SetLogLevel::Can't set log level ", toStr(level).c_str(), ", level must be between 0 and 5");
        return -1;
    }//*/
}

bool CheckLogLevel(int level){
    //#ifdef GLOGGING_ENABLED
    if(level <= DEBUG && !VLOG_IS_ON(1))
        return false;
    if(level == DEVELOP && !VLOG_IS_ON(2))
        return false;
    //#endif //GLOGGING_ENABLED
    return level >= logLevelTreshold;
}
