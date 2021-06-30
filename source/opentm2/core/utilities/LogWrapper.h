#ifndef _LOG_WRAPPER_H_
#define _LOG_WRAPPER_H_

extern "C"{
    
    enum LOGLEVEL{
        DEBUG=1,
        INFO=2,
        WARNING=3,
        ERROR=4,
        FATAL=5
    };

    enum LOGCODES{
        LOG_NO_ERROR = 0,
        LOG_SMALLER_THAN_TRESHOLD,
        LOG_FAIL_INIT,
        LOG_FAIL_OPEN_FILE
    };

    int SetLogLevel(int level);
    int LogMessage(int LogLevel, const char* message);
    int LogMessage2(int LogLevel, const char* message1, const char* message2);
    int LogMessage3(int LogLevel, const char* message1, const char* message2, const char* message3);
    int LogMessage4(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4);
    int LogMessage5(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5);
}

#endif //_LOG_WRAPPER_H_