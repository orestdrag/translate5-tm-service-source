#include "LogWrapper.h"
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

#define THREAD_ID_WRITE_TO_LOGS 1
#define CONSOLE_LOGGING 1
#define LOGERRORSINOTHERFILE 1

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

int writeLog(std::string& message, int logLevel){

    //static std::ofstream logErrorF, logF;
    #ifdef THREAD_ID_WRITE_TO_LOGS
        message += " [thread id = " + std::to_string(_getpid());
        message += "]";
    #endif
    
    if(fileLoggingSuppressed){
        message += ", (file logging suppressed)";
    }
    message += '\n';

    //write log to file
    {
        std::lock_guard<std::mutex> logGuard(logMutex); 

        #ifdef CONSOLE_LOGGING
            std::cout << message;
        #endif

        if( fileLoggingSuppressed == false){
            #ifdef LOGERRORSINOTHERFILE
                if(logLevel >= ERROR  // ONLY IMPORTANT ISSUES SHOULD BE LOGGED HERE
                        && logLevelTreshold < ERROR){ // AND ONLY IF REGULAR LOG THRESHOLD IS LESS THAN ERROR, TO AVOID DUBLICATES
                    getLogErrorFile() << message;
                    //logErrorF << message ; 
                    //logErrorF.flush();
                    //logErrorF.close();
                }
            #endif

            //if(!logF.is_open()){
            //    logF.open(logFilename, std::ios_base::app); // append instead of overwrite
            //}
            //logF << message; 
            getLogFile() << message;
            //logF.flush();
            //logF.close();
        }
    }
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
    auto logFiles = FilesystemHelper::FindFiles(logFilename + "/*.log");
    if(logFiles.size()>20){
        
        //remove files;
    }

    logFilename += "/" + generateLogFileName();
    
    initLogMsg += "Log file created, name: ";
    initLogMsg += logFilename + ", time: " + getTimeStr();
    
    desuppressLogging(prevState);
    desuppressLoggingInFile();

    return writeLog(initLogMsg, TRANSACTION);
}


int LogMessageStr(int LogLevel, const std::string& message){
    if(LogLevel<logLevelTreshold){
        return LOG_SMALLER_THAN_TRESHOLD;
    }
    if(logFilename.empty()){
        if(fileLoggingSuppressed == false){
            if( initLog() != 0)
                return LOG_FAIL_INIT;
        }
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
    case DEVELOP:
        logMessage = "[DEVELOP]";
        break;
    case TRANSACTION:
        logMessage = "[TRANSACTION]";
        break;
    default:
        break;
    }    

    logMessage += ": [" + getTimeStr() + "] :: " + message; 

    if(CheckLogLevel(DEBUG) == FALSE && logMessage.size()> 4000){
        logMessage.resize(256);
        logMessage += ". . . ";
    }

    return writeLog(logMessage, LogLevel);
}

int LogMessage(int LogLevel, std::string&& message){
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

/*
char* toStr(int i){
    //int size = snprintf(NULL, 0, "%d", i);
    //char* buffer = new char[size+1];
    //snprintf(buffer, sizeof(size+1), "%d", i);
    std::string str = std::to_string(i);
    int len = str.size();
    char* buffer = new char[len+1];
    std::strcpy(buffer, str.c_str());
    buffer[len] = '\0';
    return buffer;
}
//*/

int SetLogLevel(int level){
    if(level <= FATAL && level>=DEVELOP){
        LogMessage2(INFO, "SetLogLevel::Log level treshold was set to ",toStr(level).c_str());
        logLevelTreshold = level;
        return level;
    }else{
        LogMessage3(ERROR,"SetLogLevel::Can't set log level ", toStr(level).c_str(), ", level must be between 0 and 5");
        return -1;
    }
}

bool CheckLogLevel(int level){
    return level >= logLevelTreshold;
}
