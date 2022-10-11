#ifndef _LOG_WRAPPER_H_
#define _LOG_WRAPPER_H_

#include <string>


#define toStr(i) std::to_string(i)

//#define GLOGGING_ENABLED 1

#ifdef GLOGGING_ENABLED
#include <glog/logging.h>

#define LOG_DEBUG_MSG if(V_IS_ON(1))      LOG(INFO) <<" [DEBUG] in "<< __func__<<": "
#define LOG_INFO_MSG LOG(INFO) <<" [INFO] in "<< __func__<<": "
#define V_IS_ON(x) V_IS_ON(x)
#else 
#define V_IS_ON(x) true
#endif //GLOGGING_ENABLED


extern "C"{
 
#define LOG_DEBUG true

    enum LOGLEVEL{
        DEVELOP=0,
        DEBUG=1,
        INFO=2,
        WARNING=3,
        ERROR=4,
        FATAL=5,
        TRANSACTION=6// not an error, but for separate logs
    };

    enum LOGCODES{
        LOG_NO_ERROR = 0,
        LOG_SMALLER_THAN_TRESHOLD,
        LOG_FAIL_INIT,
        LOG_FAIL_OPEN_FILE
    };

    int SetLogLevel(int level);
    bool CheckLogLevel(int level);

    int LogMessage(int LogLevel, std::string&& message);
    int LogMessage2(int LogLevel, std::string&& message1, std::string&& message2);
    int LogMessage3(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3);
    int LogMessage4(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4);
    int LogMessage5(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5);
    int LogMessage6(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6);
    int LogMessage7(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7);
    int LogMessage8(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8);

    int LogMessage9(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8,
                        std::string&& message9);
    int LogMessage10(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8,
                        std::string&& message9, std::string&& message10);
    int LogMessage11(int LogLevel, std::string&& message1, std::string&& message2, std::string&& message3,
                    std::string&& message4, std::string&& message5, std::string&& message6, std::string&& message7, std::string&& message8,
                        std::string&& message9, std::string&& message10, std::string&& message11);
    //char* toStr(int i);

    //int SuppressLoggingInFile();
    int DesuppressLoggingInFile();
    int LogStop();

    int ResetLogBuffer();
    int SetLogBuffer(std::string logMsg);
    int AddToLogBuffer(std::string logMsg);
    void SetLogFilter(bool fFilterOn);
    std::string FlushLogBuffer();

}

#endif //_LOG_WRAPPER_H_