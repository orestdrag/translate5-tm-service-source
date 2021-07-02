#include "LogWrapper.h"
#include <string>
#include <fstream>
#include <ctime>
#include "FilesystemHelper.h"
#include <iostream>
#include "ThreadingWrapper.h"
#include <mutex>
#include <thread>

std::string logFilename;
int logLevelTreshold;

#define THREAD_ID_WRITE_TO_LOGS 1
#define CONSOLE_LOGGING 1

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

std::mutex coutMutex;   
int writeLog(std::string& message){
    #ifdef THREAD_ID_WRITE_TO_LOGS
        message += " [thread id = " + std::to_string(_getpid());
        message += "]";
    #endif

    //write log to file
    {
        std::lock_guard<std::mutex> coutLock(coutMutex); 

    #ifdef CONSOLE_LOGGING
        std::cout << message<< '\n';
    #endif
        std::ofstream logF;
        logF.open(logFilename, std::ios_base::app); // append instead of overwrite
        logF << message <<"\n"; 
        logF.close();
    }
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

int initLog(){
    int prevState = suppressLogging();
    std::string initLogMsg;
    //TODO: manage files clean-up if there more file
    logFilename = FilesystemHelper::GetOtmDir();
    logFilename += "/LOG";
    if(!FilesystemHelper::DirExists(logFilename)){
        initLogMsg += "Log directory created\n";
        FilesystemHelper::CreateDir(logFilename, 0700);
    }
    auto logFiles = FilesystemHelper::FindFiles(logFilename + "/*.log");
    if(logFiles.size()>20){
        
        //remove files;
    }

    logFilename += "/" + generateLogFileName();
    
    initLogMsg += "Log file created, name: ";
    initLogMsg += logFilename + ", time: " + getTimeStr();
    desuppressLogging(prevState);
    return writeLog(initLogMsg);
}


int LogMessageStr(int LogLevel, const std::string& message){
    if(LogLevel<logLevelTreshold){
        return LOG_SMALLER_THAN_TRESHOLD;
    }
    if(logFilename.empty()){
        if( initLog() != 0)
            return LOG_FAIL_INIT;
    }

    std::string logMessage;
    switch (LogLevel)
    {
    case FATAL:
        logMessage = "[FATAL]  ";
        break;
    case ERROR:
        logMessage = "[ERROR]  ";
        break;
    case WARNING:
        logMessage = "[WARNING]";
        break;
    case DEBUG:
        logMessage = "[DEBUG]  ";
        break;
    case INFO:
        logMessage = "[INFO]   ";
        break;
    default:
        break;
    }

    logMessage += ": [" + getTimeStr() + "] :: " + message; 

    return writeLog(logMessage);
}

int LogMessage(int LogLevel, const char* message){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, std::string(message));
}

int LogMessage2(int LogLevel, const char* message1, const char* message2){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, std::string(message1) + std::string(message2));
}

int LogMessage3(int LogLevel, const char* message1, const char* message2, const char* message3){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, std::string(message1) + std::string(message2) + std::string(message3));
}

int LogMessage4(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, std::string(message1) + std::string(message2) + std::string(message3)
                            + std::string(message4));
}
int LogMessage5(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5){
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, std::string(message1) + std::string(message2) + std::string(message3)
                            + std::string(message4) + std::string(message5));
}

int LogMessage6(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, std::string(message1) + std::string(message2) + std::string(message3)
                            + std::string(message4) + std::string(message5) + std::string(message6));
}

int LogMessage7(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6, const char* message7){
    
    return LogLevel>=logLevelTreshold && LogMessageStr(LogLevel, std::string(message1) + std::string(message2) + std::string(message3)
                            + std::string(message4) + std::string(message5) + std::string(message6) + std::string(message7));
}

char* intToA(int i){
    int size = snprintf(NULL, 0, "%d", i);
    char* buffer = new char[size+1];
    snprintf(buffer, sizeof(size+1), "%d", i);
    return buffer;
}

int SetLogLevel(int level){
    if(level <= FATAL && level>=DEBUG){
        LogMessage2(WARNING, "Log level treshold was set to ",intToA(level));
        logLevelTreshold = level;
    }
}