
#pragma once

#include <vector>
#include <string>
#include <chrono>
#include "LogWrapper.h"

#define USE_STOPWATCH 1

//use START_WATCH at the begining of the function and '}' before the end
#ifdef USE_STOPWATCH 
  #define START_WATCH { stopwatch::Stopwatch watch(__func__);
#else
  #define START_WATCH { 
#endif

#define STOP_WATCH }


#ifdef USE_STOPWATCH 
  #define PRINT_WATCH T5LOG(T5INFO) << "Request " << __func__ << " was finished in " << watch.elapsed() << " miliseconds"; 
#else
  #define PRINT_WATCH
#endif

namespace stopwatch{
    class Stopwatch{
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::string methodName;
    public:
        Stopwatch(std::string func): methodName(func){
            start_time = std::chrono::high_resolution_clock::now();
        }

        std::uint64_t elapsed(){
            const auto duration = std::chrono::high_resolution_clock::now() - start_time;
            const std::uint64_t ns_count = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
            std::uint64_t up = ((ns_count/100000)%10 >= 5) ? 1 : 0;
            const auto ms_count = (ns_count /1000000) + up;
        }
        std::string print(){
            return /*"{" + */ std::to_string(elapsed()) + /*"}*/" ms\"";
        }
        ~Stopwatch(){
            T5LOG(T5INFO) << "stopwatch, method  = " << methodName << "; time = " << print();
        }
    
    };


}