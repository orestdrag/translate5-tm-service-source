#ifndef _LOG_WRAPPER_H_
#define _LOG_WRAPPER_H_


extern "C"{
    
    enum LOGLEVEL{
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };
    enum LOGCODES{
        LOG_NO_ERROR = 0,
        LOG_FAIL_INIT,
        LOG_FAIL_OPEN_FILE
    };
    int LogMessage(int LogLevel, const char* message);
}

#endif //_LOG_WRAPPER_H_