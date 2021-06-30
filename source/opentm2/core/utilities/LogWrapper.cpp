#include "LogWrapper.h"
#include <string>
#include <fstream>
#include <ctime>
#include "FilesystemHelper.h"
#include <iostream>

std::string logFilename;

std::string getDateStr(){
    // current date/time based on current system
   time_t now = time(0);
   
   // convert now to string form
   std::string sDate = ctime(&now);
   int nLinePos = sDate.find("\n");
    if(nLinePos != std::string::npos)
        sDate[nLinePos] = '\0';
   return sDate;
}

std::string getTimeStr(){
    // current date/time based on current system
    time_t now = time(0);
   
    // convert now to string form
    std::string sTime =  ctime(&now);
    int nLinePos = sTime.find("\n");
    if(nLinePos != std::string::npos)
        sTime[nLinePos] = '\0';
    return sTime;
}

std::string generateLogFileName(){
    // current date/time based on current system
   time_t now = time(0);
   
   // convert now to string form
   std::string fName = "Log_" + getDateStr() + ".log";
   return fName;
}

int writeLog(const std::string& message){
    //write log to file
    std::ofstream logF;
    logF.open(logFilename, std::ios_base::app); // append instead of overwrite
    logF << message <<"\n"; 
    logF.close();

//#ifdef DEBUG
    std::cout << message<< '\n';
//#endif

    return 0;
}

int initLog(){
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
    return writeLog(initLogMsg);
}

int LogMessage(int LogLevel, const char* message){
    if(logFilename.empty()){
        if( initLog() != 0)
            return LOG_FAIL_INIT;
    }

    std::string logMessage;
    switch (LogLevel)
    {
    case FATAL:
        logMessage = "[FATAL]";
        break;
    case ERROR:
        logMessage = "[ERROR]";
        break;
    case WARNING:
        logMessage = "[WARNING]";
        break;
    case DEBUG:
        logMessage = "[DEBUG]";
        break;
    case INFO:
        logMessage = "[INFO]";
        break;
    default:
        break;
    }

    logMessage += ": [" ;
    logMessage += getTimeStr(); 
    logMessage += "] :: ";
    logMessage += message;

    writeLog(logMessage);
}
