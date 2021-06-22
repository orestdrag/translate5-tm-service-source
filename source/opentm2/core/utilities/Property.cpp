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
    return properties.init();
}

void properties_deinit() {
    properties.deinit();
}

int properties_add_str(const char* key, const char* value) {
    return properties.add_key(key, value);
}

int properties_set_str(const char* key, const char* value) {
    return properties.set_value(key, value);
}

int properties_add_int(const char* key, const int value) {
    return properties.add_key(key, value);
}

int properties_set_int(const char* key, const int value) {
    return properties.set_value(key, value);
}

int properties_get_str(const char *key, char *buff, int buffSize ){
    std::string str;
    int res = properties.get_value(key, str);
    
    if(res == Properties::NO_ERRORS)
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
    return properties.add_key(key, value);
}

int properties_set_value(const char *key, int value) {
    return properties.set_value(key, value);
}


char * properties_get_otm_dir() {
    std::string otm_dir = properties.get_otm_dir();
    char *otmdir = (char *)malloc(otm_dir.size() + 1);
    if (otmdir)
        strncpy(otmdir, otm_dir.c_str(), otm_dir.size());
    return otmdir;
}

/* Implementation */

int Properties::init() {
    if (get_home_dir())
        return ERROR_FILE_CANT_GET_HOME_DIR;

    otm_dir = home_dir + "/." + OTMMEMORYSERVICE;

    if (create_otm_dir())
        return ERROR_FILE_CANT_CREATE_OTM_DIRECTORY;

    filename = otm_dir + "/" + "Properties";
    if (read_all_data_from_file() == ERROR_FILE_CANT_OPEN){
        return create_properties_file();
    }

    read_all_data_from_file();
    return NO_ERRORS;
}

void Properties::deinit() {
    //dataStr.clear();
    //dataInt.clear();
    fs.close();
}

int Properties::add_key(const std::string& key, const std::string& value) {
    if (exist_string(key) == NO_ERRORS){
        return ERROR_STR_KEY_ALREADY_EXISTS;
    }
    dataStr.insert({ key, value });
    if (int writeDataReturn = update_strData_in_file(key))
        return writeDataReturn;

    return NO_ERRORS;
}

int Properties::add_key(const std::string& key, const int value) {
    if (exist_int(key) == NO_ERRORS){
        return ERROR_INT_KEY_ALREADY_EXISTS;
    }

    dataInt.insert({ key, value });
    if (int writeDataReturn = update_strData_in_file(key))
        return writeDataReturn;

    return NO_ERRORS;
}

int Properties::set_value(const std::string& key, const std::string& value) {
    if(int existRet = exist_string(key)){
        return existRet;
    }

    dataStr.at(key) = value;

    int writeDataReturn = update_strData_in_file(key);
    return writeDataReturn;
}

int Properties::set_value(const std::string& key, const int value) { 
    
    if(int existRet = exist_int(key)){
        return existRet;
    }

    dataInt.at(key) = value;
    
    int writeDataReturn = update_intData_in_file(key);
    return writeDataReturn;
}

int Properties::get_value(const std::string& key, std::string& value){
    if(int existRet = exist_string(key)){
        return existRet;
    }
    value = dataStr.at(key);
    return NO_ERRORS;    
}

int Properties::get_value(const std::string& key, int& value){
    if(int existRet = exist_int(key)){
        return existRet;
    }

    value = dataInt.at(key);
    return NO_ERRORS;    
}


bool Properties::exist_int_in_map(const std::string& key){
    return dataInt.count(key);
}

bool Properties::exist_string_in_map(const std::string& key){
    return dataStr.count(key);
}

bool Properties::exist_in_map(const std::string& key){
    return exist_string_in_map(key) || exist_int_in_map(key);
}

bool Properties::exist(const std::string& key){
    return exist_string(key)==NO_ERRORS || exist_int(key)==NO_ERRORS;
}

int Properties::exist_int(const std::string& key){
    if (!exist_int_in_map(key)){ // if we don't hame key in memory-trying to load from file
        int updateRes = update_intData_from_file(key);
        if(updateRes != NO_ERRORS){
            return updateRes;
        }
        if(!exist_int_in_map(key)){
            return ERROR_INT_KEY_NOT_EXISTS;
        }
    }
    return NO_ERRORS;
}
int Properties::exist_string(const std::string& key){
    if (!exist_string_in_map(key)){ // if we don't hame key in memory-trying to load from file
        int updateRes = update_strData_from_file(key);
        if(updateRes != NO_ERRORS){
            return updateRes;
        }
        if(!exist_string_in_map(key)){
            return ERROR_INT_KEY_NOT_EXISTS;
        }
    }
    return NO_ERRORS;
}

std::string Properties::get_otm_dir() const {
    return otm_dir;
}

int Properties::get_home_dir() {
    home_dir = getenv(HOME_ENV);
    if (!home_dir.empty())
        return  NO_ERRORS;

    struct passwd *pswd = getpwuid(getuid());
    if (!pswd)
        return ERROR_FILE_CANT_GET_USER_PSWD;
    home_dir = pswd->pw_dir;

    return NO_ERRORS;
}

int Properties::create_otm_dir() {
    struct stat st;
    int ret = stat(otm_dir.c_str(), &st);
    if (!ret)
        return NO_ERRORS;
    return mkdir(otm_dir.c_str(), 0700);
}

int Properties::create_properties_file(){
    fs.open(filename, std::ios::binary | std::ios::out | std::ofstream::trunc);
    fs << "<stringProperties>\n</stringProperties>\n";
    fs << "<intProperties>\n</intProperties>\n";
    fs.close();
    return NO_ERRORS;
}

int Properties::read_all_data_from_file() {
    std::string line;
    std::string::size_type n;
    const char delim = '=';

    fs.open(filename, std::ios::binary | std::ios::in);
    if (!fs.is_open())
        return ERROR_FILE_CANT_OPEN;

    dataStr.clear();
    dataInt.clear();

    std::string beginRegion = "<stringProperties>";
    std::string endRegion = "</stringProperties>";

    //searching for begining of region
    while(std::getline(fs, line) && line != beginRegion);

    if(fs.eof())
        return ERROR_FILE_STRINGPROPERTIES_NOT_FOUND;
    
    std::getline(fs, line);
    while (line != endRegion) {
        n = line.find(delim);
        if (n != std::string::npos)
            dataStr.insert({ line.substr(0, n), line.substr(n + 1,
                                            std::string::npos) });
        std::getline(fs, line);
    }

    beginRegion = "<intProperties>";
    endRegion = "</intProperties>";
    
    //searching for begining of region
    while(std::getline(fs, line) && line != beginRegion);
    
    if(fs.eof())
        return ERROR_FILE_INTPROPERTIES_NOT_FOUND;

    std::getline(fs, line);
    while (line != endRegion) {
        n = line.find(delim);
        if (n != std::string::npos){
            int num = std::stoi(line.substr(n + 1, std::string::npos));
            dataInt.insert({ line.substr(0, n), num });
        }
    }

    fs.close();
    return NO_ERRORS;
}

int Properties::write_all_data_to_file() {
    fs.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    
    if (!fs.is_open())
        return ERROR_FILE_CANT_OPEN;

    fs << "<stringProperties>\n";
    for (auto it = dataStr.begin(); it != dataStr.end(); ++it)
        fs << it->first << "=" << it->second << "\n";
    fs << "</stringProperties>\n";
    
    fs << "<intProperties>\n";
    for (auto it = dataInt.begin(); it != dataInt.end(); ++it)
        fs << it->first << "=" << std::to_string(it->second) << "\n";
    fs << "</intProperties>\n";

    fs.close();
    return NO_ERRORS;
}

int Properties::update_intData_from_file(const std::string& key){
    //TODO rewrite for better optimization
    read_all_data_from_file();
    if(!exist_int_in_map(key))
        return ERROR_FILE_KEY_NOT_FOUND;
    return NO_ERRORS;
}

int Properties::update_strData_from_file(const std::string& key){
    //TODO rewrite for better optimization
    read_all_data_from_file();
    if(!exist_string_in_map(key))
        return ERROR_FILE_KEY_NOT_FOUND;
    return NO_ERRORS;
}

int Properties::update_intData_in_file(const std::string& key){
    //TODO rewrite to update only needed data in file
    write_all_data_to_file();
    return NO_ERRORS;
}
    
int Properties::update_strData_in_file(const std::string& key){
    //TODO rewrite to update only needed data in file
    write_all_data_to_file();
    return NO_ERRORS;
}
