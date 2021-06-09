#include "Property.h"
#include "PropertyWrapper.H"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <cstdio>
#include <cstring>

#define HOME_ENV "HOME"
#define OTMMEMORYSERVICE "OtmMemoryService"

/* Wrapper functions for C code usage */

static Properties properties;

int properties_init() {
    if (properties.init())
        return -1;
    return 0;
}

void properties_deinit() {
    properties.deinit();
}

int properties_add_key(const char *key, const char *value) {
    if (properties.add_key(key, value))
        return -1;
    return 0;
}

int properties_set_value(const char *key, const char *value) {
    if (properties.set_value(key, value))
        return -1;
    return 0;
}

char * properties_get_otm_dir() {
    std::string otm_dir = properties.get_otm_dir();
    char *otmdir = (char *)malloc(otm_dir.size() + 1);
    if (!otmdir)
        return NULL;
    strncpy(otmdir, otm_dir.c_str(), otm_dir.size());
    return otmdir;
}

/* Implementation */

int Properties::init() {
    if (get_home_dir())
        return -1;

    otm_dir = home_dir + "/." + OTMMEMORYSERVICE;

    if (create_otm_dir())
        return -1;

    filename = otm_dir + "/" + "Properties";

    return 0;
}

void Properties::deinit() {
    data.clear();
    fs.close();
}

int Properties::add_key(const std::string& key, const std::string& value) {
    read_data();

    if (data.count(key))
        return 1;

    data.insert({ key, value });
    if (write_data())
        return -1;

    return 0;
}

int Properties::set_value(const std::string& key, const std::string& value) {
    read_data();

    if (!data.count(key))
        return -1;

    data.at(key) = value;
    if (write_data())
        return -1;

    return 0;
}

std::string Properties::get_otm_dir() const {
    return otm_dir;
}

int Properties::get_home_dir() {
    home_dir = getenv(HOME_ENV);
    if (!home_dir.empty())
        return 0;

    struct passwd *pswd = getpwuid(getuid());
    if (!pswd)
        return -1;
    home_dir = pswd->pw_dir;

    return 0;
}

int Properties::create_otm_dir() {
    struct stat st;
    int ret = stat(otm_dir.c_str(), &st);
    if (!ret)
        return 0;
    return mkdir(otm_dir.c_str(), 0700);
}

int Properties::read_data() {
    std::string line;
    std::string::size_type n;
    const char delim = '=';

    fs.open(filename, std::ios::binary | std::ios::in);
    if (!fs.is_open())
        return -1;

    data.clear();

    while (std::getline(fs, line)) {
        n = line.find(delim);
        if (n != std::string::npos)
            data.insert({ line.substr(0, n), line.substr(n + 1,
                                            std::string::npos) });
    }

    fs.close();
    return 0;
}

int Properties::write_data() {
    fs.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!fs.is_open())
        return -1;

    for (auto it = data.begin(); it != data.end(); ++it)
        fs << it->first << "=" << it->second << "\n";

    fs.close();
    return 0;
}
