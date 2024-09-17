#include "LogWrapper.h"
#include <execinfo.h>
//#include <stacktrace>


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

T5Logger* T5Logger::GetInstance(){
    static T5Logger _instance;
    return &_instance;
}

std::ostringstream& T5Logger::__getLogBuffer(){
    //static std::ostringstream __logBuff;
    thread_local std::ostringstream __logBuff;
    return __logBuff;
}

std::ostringstream& T5Logger::__getBodyBuffer(){
    //static std::ostringstream __logBuff;

    thread_local std::ostringstream __logBuff;
    return __logBuff;
}

std::string getBackTrace(){
    void *array[50];
    char **strings;
    int size, i;

    size = backtrace (array, 50);
    strings = backtrace_symbols (array, size);
    std::string res;
    if (strings != NULL)
    {
        //printf ("Obtained %d stack frames.\n", size);
        for (i = 0; i < size; i++){
            res += strings[i];
            res += '::';
        //printf ("%s\n", strings[i]);
        }
        free (strings);
    }
    return res;
}

int T5Logger::LogStop(){
    //T5LOG(TRANSACTION) << " called LogStop -> closing log files";
    return 0;
}

int T5Logger::DesuppressLoggingInFile(){
    //desuppressLoggingInFile();
    //initLog();
    return 0;
}

int T5Logger::suppressLogging(){
    int prevTreshold = logLevelTreshold;
    logLevelTreshold = T5FATAL + 1;
    return prevTreshold;
}
int T5Logger::desuppressLogging(int prevTreshold){
    return logLevelTreshold = prevTreshold;
}

int T5Logger::ResetLogBuffer(){
    __getLogBuffer().str("");
    __getBodyBuffer().str("");
    return 0;
}

int T5Logger::SetLogBuffer(std::string logMsg){
    ResetLogBuffer();
    __getLogBuffer() << logMsg << std::endl;
    return 0;
}

int T5Logger::SetLogInfo(int RequestType){
    requestType = RequestType;
    return 0;
}

int T5Logger::AddToLogBuffer(std::string logMsg){
    __getLogBuffer() << logMsg;
    return 0;
}

int T5Logger::SetBodyBuffer(std::string logMsg){
    __getBodyBuffer().str(logMsg);
    auto str = __getBodyBuffer().str();
    return 0;
}

std::string T5Logger::FlushLogBuffer(){
    auto str = __getLogBuffer().str();
    __getLogBuffer().str("");
    return str;
}

std::string T5Logger::FlushBodyBuffer(){
    auto str = __getBodyBuffer().str();
    __getBodyBuffer().str("");
    return str;
}

int T5Logger::SetLogLevel(int level){
    if(level <= T5FATAL && level>= T5DEVELOP){
        T5LOG(T5INFO) << "SetLogLevel::Log level treshold was set to " << level;
        logLevelTreshold = level;
        return level;
    }else{
        T5LOG(T5ERROR) << "SetLogLevel::Can't set log level " << level << ", level must be between 0 and 5";
        return -1;
    }
}

int T5Logger::GetLogLevel(){
    return logLevelTreshold;
}
bool T5Logger::CheckLogLevel(int level){
    if(level <= T5DEBUG && !VLOG_IS_ON(1))
        return false;
    if(level == T5DEVELOP && !VLOG_IS_ON(2))
        return false;
    return level >= logLevelTreshold;
}

void T5Logger::SetLogFilter(bool filterOn){
    fFilterLogs = filterOn;
}

std::string T5Logger::FlushBuffers(int severity){
    if(severity >= T5ERROR){
        auto LogBuffer = T5Logger::GetInstance()->FlushLogBuffer();
        auto BodyBuffer = T5Logger::GetInstance()->FlushBodyBuffer();
        if(BodyBuffer.size() > 3000) BodyBuffer = BodyBuffer.substr(0, 3000);
        if(LogBuffer.size() > 3000) LogBuffer = LogBuffer.substr(0, 3000);
        if(LogBuffer.empty()) return BodyBuffer;
        return BodyBuffer + "\n\tlb: { " + LogBuffer + "}\n";
    }
    return "";
}

int getBuffIdForLog(int severity){
    if(T5Logger::GetInstance()->logLevelTreshold > severity)     
        return -1; //NullStream
    if( !T5Logger::GetInstance()->GetInstance()->fFilterLogs || VLOG_IS_ON(2) || severity >= T5ERROR ){
        return severity;
    }   
    if( VLOG_IS_ON(1) )
        return -2; //T5Logger::GetInstance()->__getLogBuffer(); 
    return  -1; //NullStream
}



