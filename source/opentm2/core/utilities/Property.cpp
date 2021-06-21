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

int properties_add_str(const char* key, const char* value) {
    if (properties.add_key(key, value))
        return -1;
    return 0;
}

int properties_set_str(const char* key, const char* value) {
    if (properties.set_value(key, value))
        return -1;
    return 0;
}

int properties_add_int(const char* key, const int value) {
    if (properties.add_key(key, value))
        return -1;
    return 0;
}

int properties_set_int(const char* key, const int value) {
    if (properties.set_value(key, value))
        return -1;
    return 0;
}

int properties_get_str(const char *key, char *buff, int buffSize ){
    std::string str;
    int res = properties.get_value(key, str);
    strncpy(buff, str.c_str(), buffSize);
    return res;
}

int properties_get_int(const char * key, int& value ){
    return properties.get_value(key, value);
}

bool properties_exist(const char * key){
    return properties.exist(key);
}

int properties_add_key(const char *key, int value) {
    if (properties.add_key(key, value))
        return -1;
    return 0;
}

int properties_set_value(const char *key, int value) {
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
    dataStr.clear();
    dataInt.clear();
    fs.close();
}

int Properties::add_key(const std::string& key, const std::string& value) {
    read_data();

    if (dataStr.count(key))
        return 1;

    dataStr.insert({ key, value });
    if (write_data())
        return -1;

    return 0;
}

int Properties::add_key(const std::string& key, const int value) {
    read_data();

    if (existInt(key))
        return 1;

    dataInt.insert({ key, value });
    if (write_data())
        return -1;

    return 0;
}

int Properties::set_value(const std::string& key, const std::string& value) {
    read_data();

    if (!existString(key))
        return -1;

    dataStr.at(key) = value;
    if (write_data())
        return -1;

    return 0;
}

int Properties::set_value(const std::string& key, const int value) {
    read_data();

    if (existInt(key))
        return -1;

    dataInt.at(key) = value;
    if (write_data())
        return -1;

    return 0;
}

int Properties::get_value(const std::string& key, std::string& value){
    read_data();

    if(!existString(key))
        return -1;

    value = dataStr.at(key);
    return 0;    
}

int Properties::get_value(const std::string& key, int& value){
    read_data();

    if(!existInt(key))
        return -1;

    value = dataInt.at(key);
    return 0;    
}


bool Properties::existInt(const std::string& key){
    return dataInt.count(key);
}

bool Properties::existString(const std::string& key){
    return dataStr.count(key);
}

bool Properties::exist(const std::string& key){
    return existString(key) || existInt(key);
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

    dataStr.clear();

    while (std::getline(fs, line)) {
        n = line.find(delim);
        if (n != std::string::npos)
            dataStr.insert({ line.substr(0, n), line.substr(n + 1,
                                            std::string::npos) });
    }

    dataInt.clear();

    while (std::getline(fs, line)) {
        n = line.find(delim);
        if (n != std::string::npos){
            int num = std::stoi(line.substr(n + 1, std::string::npos));
            dataInt.insert({ line.substr(0, n), num });
        }
    }

    fs.close();
    return 0;
}

int Properties::write_data() {
    fs.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!fs.is_open())
        return -1;

    for (auto it = dataStr.begin(); it != dataStr.end(); ++it)
        fs << it->first << "=" << it->second << "\n";

    for (auto it = dataInt.begin(); it != dataInt.end(); ++it)
        fs << it->first << "=" << it->second << "\n";
    fs.close();
    return 0;
}
