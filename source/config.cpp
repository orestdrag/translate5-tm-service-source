#include "config.h"
#include <map>
#include <fstream>
#include <iostream>
#include "./opentm2/core/utilities/LogWrapper.h"

int config::parse() {
    std::fstream file(filename);
    if (!file) {
        //std::cerr << "Failed to open config file\n";
        LogMessage(T5ERROR, __func__,":: Failed to open config file ", filename.c_str());
        return -1;
    }
    LogMessage( T5INFO, __func__,":: Success in opening config file ", filename.c_str());
    std::string line;
    std::string::size_type n;
    const char delim = '=';

    while (getline(file, line)) {
        n = line.find(delim);
        if (n != std::string::npos)
            data.insert({ line.substr(0, n), line.substr(n + 1, std::string::npos) });
    }

    return 0;
}
