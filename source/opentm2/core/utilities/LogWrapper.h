#ifndef _LOG_WRAPPER_H_
#define _LOG_WRAPPER_H_

extern "C"{
    

#define LOG_DEBUG true

    enum LOGLEVEL{
        DEVELOP=0,
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
    bool CheckLogLevel(int level);
    int LogMessage(int LogLevel, const char* message);
    int LogMessage2(int LogLevel, const char* message1, const char* message2);
    int LogMessage3(int LogLevel, const char* message1, const char* message2, const char* message3);
    int LogMessage4(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4);
    int LogMessage5(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5);
    int LogMessage6(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6);
    int LogMessage7(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6, const char* message7);
    int LogMessage8(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6, const char* message7, const char* message8);

    int LogMessage9(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6, const char* message7, const char* message8,
                        const char* message9);
    int LogMessage10(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6, const char* message7, const char* message8,
                        const char* message9, const char* message10);
    int LogMessage11(int LogLevel, const char* message1, const char* message2, const char* message3,
                    const char* message4, const char* message5, const char* message6, const char* message7, const char* message8,
                        const char* message9, const char* message10, const char* message11);
    char* intToA(int i);

    //int SuppressLoggingInFile();
    int DesuppressLoggingInFile();
}

#endif //_LOG_WRAPPER_H_