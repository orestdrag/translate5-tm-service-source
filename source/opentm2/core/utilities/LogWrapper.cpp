#include "LogWrapper.h"



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

//*
//std::ostringstream& T5Logger::__getLogBuffer(){
    //static std::ostringstream __logBuff;
    //return __logBuff;
//}
std::ostream& T5Logger::__getLogBuffer(){
    //static std::ostringstream __logBuff;
    //return __logBuff;
    return T5Logger::GetInstance()->__googleBuff->stream();
}
/*
std::ostringstream& T5Logger::getLogBuffer(){
    //__getLogBuffer() <<"\t{lb}: ";
    auto str = __getLogBuffer().str();
    return __getLogBuffer();
}//*/

std::ostringstream& T5Logger::__getBodyBuffer(){
    static std::ostringstream __logBuff;
    return __logBuff;
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
    //__getLogBuffer().str("");
    __getLogBuffer().flush();
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
    std::ostringstream text;
    text << __getLogBuffer().rdbuf();
    auto str = text.str();
    __getLogBuffer().flush();
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

std::ostream& getBuffForLog(int severity){
    if(T5Logger::GetInstance()->logLevelTreshold > severity)     
        return T5Logger::GetInstance()->nullStream;
    if( !T5Logger::GetInstance()->GetInstance()->fFilterLogs || VLOG_IS_ON(2) || severity>=T5ERROR ){
        if(severity == T5DEVELOP)       return _T5_LOG_T5DEVELOP;
        else if(severity == T5DEBUG)    return _T5_LOG_T5DEBUG;
        else if(severity == T5INFO)     return _T5_LOG_T5INFO;
        else if(severity == T5WARNING)  return _T5_LOG_T5WARNING;
        else if(severity == T5ERROR)    return _T5_LOG_T5ERROR;
        else if(severity == T5FATAL)    return _T5_LOG_T5DEVELOP;
        else                            return _T5_LOG_T5TRANSACTION;
    }   
    if( !VLOG_IS_ON(1) )
        return T5Logger::GetInstance()->nullStream;
    return  T5Logger::GetInstance()->__getLogBuffer();
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


int LogMessageStr(int LogLevel, std::string message){ 
                    switch(LogLevel){
                        case T5TRANSACTION:
                            T5LOG(T5TRANSACTION) << message;
                        break;
                        case T5FATAL:
                            T5LOG(T5FATAL) << message;
                        break;
                        case T5ERROR:
                            T5LOG(T5ERROR) << message;
                        break;
                        case T5WARNING:
                            T5LOG(T5WARNING) << message;
                        break;
                        case T5INFO:
                            T5LOG(T5INFO) << message;
                        break;
                        case T5DEBUG:
                            T5LOG(T5DEBUG) << message;
                        break;
                        case T5DEVELOP:
                            T5LOG(T5DEVELOP) << message;
                        break;
                        default:
                        break;
                    }
                    return 0; 
        }


int LogMessage(int LogLevel, std::string message1){ 
                    LogMessageStr(LogLevel, message1);
                    return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2){ 
                    LogMessageStr(LogLevel, message1 + message2);
                    return 0; }

int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3){ 
                    LogMessageStr(LogLevel, message1 + message2 + message3);
                    return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4){ 
                    LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4  );  
                        return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5){ 
                    LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4 + message5 );  
                        return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6){ 
                    LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4 + message5 + message6  );  
                    return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7){ 
                    LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4 + message5 + message6 + message7 );                        
                        return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8){ 
                    LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4 + message5 + message6 + message7 
                        + message8);
                        return 0; }

int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8,
                    std::string message9){ 
                        LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4 + message5 + message6 + message7 
                        + message8 + message9);
                        return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8,
                    std::string message9, std::string message10) { 
                        LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4 + message5 + message6 + message7 
                        + message8 + message9 + message10);
                        return 0; }
int LogMessage(int LogLevel, std::string message1, std::string message2, std::string message3,
                std::string message4, std::string message5, std::string message6, std::string message7, std::string message8,
                    std::string message9, std::string message10, std::string message11) { 
                        LogMessageStr(LogLevel, message1 + message2 + message3
                        + message4 + message5 + message6 + message7 
                        + message8 + message9 + message10 + message11);
                        return 0;
                     }

