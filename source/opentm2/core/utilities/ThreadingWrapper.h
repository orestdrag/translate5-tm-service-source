#ifndef _THREADING_WRAPPER_H_
#define _THREADING_WRAPPER_H_

const int MUTEX_ALL_ACCESS = 0x1F0001;
const int INFINITE = -2;

int _getpid();
double getCurrentCPUUsageByProcess();
int getVirtualMemUsageKBValue();
void mem_usage(double& vm_usage, double& resident_set);


#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <string>
#include "LogWrapper.h"

class MutexTimeout{
    size_t m_timeout_ms = 0;
    bool m_timeout_failed = false;
    std::string m_errMsg;
public:
    MutexTimeout(size_t timeout_ms):m_timeout_ms(timeout_ms){}
    MutexTimeout(){}

    std::string getErrMsg()const { return m_errMsg;}

    //void addToErrMsg(const char* msg) { 
    //    m_errMsg += msg; 
    //    m_errMsg +="\n"; 
    //}

    void addToErrMsg(const char* msg, const char* func, int line) {
        m_timeout_failed = true;
        m_errMsg += msg; 
        m_errMsg += "(location: " + std::string(func) +":"+ std::to_string(line) + "); ";
    }

    size_t getTimeout_ms()const {return m_timeout_ms; }
    void setTimeout_ms(size_t timeout_ms){m_timeout_ms = timeout_ms; }

    bool failed() const{
        return m_timeout_failed;
    }

    void reset(){
        m_timeout_failed = false;
        m_errMsg = "";
        m_timeout_ms = 0;
    }
};

DECLARE_bool(logMutexes);
#define MUTEX_LOG T5LOG(T5TRANSACTION)

class TimedMutexGuard {
    int64_t m_id = 0;
public:
    static int64_t _id_ ;

    // Constructor: Takes a reference to the mutex and a timeout duration
    TimedMutexGuard(std::recursive_timed_mutex& mtx, MutexTimeout& timeout, const char* addInfo, const char* func,  int line)
        : m_mtx(mtx), m_owns_lock(false)
    {
        m_id = ++(TimedMutexGuard::_id_);
        if(timeout.failed()){
            mutexInfo = "Timeout was already failed for the lock("+ std::string(addInfo) 
                    +") within the timeout(" + std::to_string(timeout.getTimeout_ms())+"ms)";
            timeout.addToErrMsg(mutexInfo.c_str(), func, line);
            mutexInfo += ", position:"  + std::string(func) +":", std::to_string(line); 
            if(FLAGS_logMutexes){
                T5LOG(T5ERROR) << mutexInfo;
            }

        }
        else if (0 == timeout.getTimeout_ms()) {
            if(FLAGS_logMutexes){
                MUTEX_LOG << "Requesting non-timed mutex " << addInfo <<", id =" << m_id << " from " << func<<":" <<line;
            }
            // If timeout is 0, just lock the mutex immediately (blocking)
            m_mtx.lock();
            if(FLAGS_logMutexes){
                MUTEX_LOG << "non-timed mutex locked " << addInfo <<", id =" << m_id <<  " from " << func<<":" <<line;
            }
            m_owns_lock = true;
            m_timed = false;
        } else {
            if(FLAGS_logMutexes){
               MUTEX_LOG << "Requesting timed mutex " << addInfo <<", id =" << m_id <<  " from " << func<<":" <<line << "; with timeout = "<< timeout.getTimeout_ms();
            }
            // Try to lock the mutex within the specified timeout duration
            m_owns_lock = m_mtx.try_lock_for(std::chrono::milliseconds(timeout.getTimeout_ms()));
            m_timed = true;
    
            if (!m_owns_lock) {
                mutexInfo = "Failed to acquire the lock("+ std::string(addInfo) 
                    +":"+ std::to_string(m_id)+") within the timeout(" + std::to_string(timeout.getTimeout_ms())+"ms)";
                    
                timeout.addToErrMsg(mutexInfo.c_str(), func, line);
                mutexInfo += ", position:"  + std::string(func) +":", std::to_string(line); 
                if(FLAGS_logMutexes){
                    T5LOG(T5ERROR) << mutexInfo;
                }
            }
        }
    }

    // Destructor: Automatically unlock the mutex if it was locked
    ~TimedMutexGuard() {
        if (m_owns_lock) {
            if(FLAGS_logMutexes){
                if(m_timed){
                    MUTEX_LOG << "Unlocking timed mutex, id = " << m_id;
                }else{
                    MUTEX_LOG << "Unlocking non-timed mutex, id = " << m_id;
                }
            }
            m_mtx.unlock();
        }
    }

    // Check if the lock was successfully acquired
    bool owns_lock() const {
        return m_owns_lock;
    }

    bool timed() const{
        return false;
    }

    std::string getMutexInfo(){
        return mutexInfo;
    }

    // Deleted copy constructor and assignment operator to prevent copying
    TimedMutexGuard(const TimedMutexGuard&) = delete;
    TimedMutexGuard& operator=(const TimedMutexGuard&) = delete;

private:
    std::recursive_timed_mutex& m_mtx;
    bool m_owns_lock = false;
    bool m_timed = false;

    std::string mutexInfo;
};

#endif
