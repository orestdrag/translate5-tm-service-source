#include "config.h"
#include <map>
#include <fstream>
#include <iostream>

int config::parse() {
    std::fstream file(filename);
    if (!file) {
        std::cerr << "Failed to open config file\n";
        return -1;
    }

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
