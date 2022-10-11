#include "Property.h"
#include "PropertyWrapper.H"

#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdlib.h>

#include "EQF.H"
#include "FilesystemWrapper.h"
#include "FilesystemHelper.h"
#include "LogWrapper.h"

#define HOME_ENV "HOME"
#define OTMMEMORYSERVICE "t5memory"

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

void properties_turn_on_saving_in_file(){
    properties.set_write_to_file(true);
}

void properties_turn_off_saving_in_file(){
    properties.set_write_to_file(false);
}

bool properties_set_str_anyway(const char* key, const char* value){
    return properties.set_anyway(key, value) == PROPERTY_NO_ERRORS;
}

bool properties_set_int_anyway(const char* key, const int value){
    return properties.set_anyway(key, value) == PROPERTY_NO_ERRORS;
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
    
    if(res == PROPERTY_NO_ERRORS)
        strncpy(buff, str.c_str(), buffSize);
    return res;
}

int properties_get_int(const char * key, int& value ){
    return properties.get_value(key, value);
}

int properties_get_int_or_default(const char* key, int& value, const int defaultValue){
    int res = properties_get_int(key, value);
    if(res != PROPERTY_NO_ERRORS){
        value = defaultValue;
        properties_add_int(key, defaultValue);
        if(V_IS_ON(1)){
            LogMessage4(DEVELOP, "Key ", key , " not saved yet -> saving default value : ", toStr(defaultValue).c_str());
        }
        return PROPERTY_USED_DEFAULT_VALUE;
    }
    return PROPERTY_NO_ERRORS;
}


int properties_get_str_or_default(const char* key, char *buff, int buffSize, const char* defaultValue ){
     int res = properties_get_str(key, buff, buffSize);
    if(res != PROPERTY_NO_ERRORS){
        if(buff != defaultValue){
            strncpy(buff, defaultValue, buffSize);
        }
        properties_add_str(key, defaultValue);
        if(V_IS_ON(2)){
            LogMessage4(DEVELOP, "Key ", key , " not saved yet -> saving default value : ", defaultValue);
        }
        return PROPERTY_USED_DEFAULT_VALUE;
    }
    return PROPERTY_NO_ERRORS;
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

/* Implementation */

int Properties::init() {
    //LOG_DEVELOP_MSG << "Properties::init()";
    
    LogMessage(DEVELOP, "Properties::init()");
    //errCode here can be not NO_ERROR, because properties files path not setuped yet, so it couldn't save data yet
    int errCode = init_home_dir_prop();

    std::string home_dir; 
    errCode = get_value(KEY_HOME_DIR, home_dir);
    
    std::string otm_dir = home_dir + "/." + OTMMEMORYSERVICE;

    if (FilesystemHelper::CreateDir(otm_dir)){
        LogMessage2(ERROR, "PROPERTY_ERROR_FILE_CANT_CREATE_OTM_DIRECTORY, otm_dir: ", otm_dir.c_str());
        return PROPERTY_ERROR_FILE_CANT_CREATE_OTM_DIRECTORY;
    }
    set_value(KEY_OTM_DIR, otm_dir);
    std::string mem_dir = otm_dir+ "/MEM/";
    set_value(KEY_MEM_DIR, mem_dir);
    
    /*
    std::string plugin_dir = otm_dir + "/PLUGINS/";
    if(FilesystemHelper::CreateDir(plugin_dir)){
        LogMessage2(ERROR, "PROPERTY_ERROR_FILE_CANT_CREATE_PLUGIN_DIR, plugin_dir: ", plugin_dir.c_str());
        return PROPERTY_ERROR_FILE_CANT_CREATE_PLUGIN_DIR;
    }
    set_anyway(KEY_PLUGIN_DIR, plugin_dir);
    //*/

    filename_str = otm_dir + "/" + "Properties_str";
    filename_int = otm_dir +"/" + "Properties_int";
    if (read_all_data_from_file() == PROPERTY_ERROR_FILE_CANT_OPEN){
        if(V_IS_ON(1)){
            LogMessage5(INFO, "PROPERTY_ERROR_FILE_CANT_OPEN, filename_int: ", 
            filename_int.c_str(), "; filename_str: ", filename_str.c_str(), ", try creating new");
        }
        if( errCode = create_properties_file()){
            LogMessage2(ERROR, "Failed to create properties file, errCode = ", std::to_string(errCode).c_str());
        }
    }
    if(V_IS_ON(2)){
        LogMessage4(DEVELOP, "Properties::init done, filename_str = ",filename_str.c_str(), ", filename_int = ", filename_int.c_str() );
    }
    return PROPERTY_NO_ERRORS;
}

void Properties::deinit() {
    LogMessage(INFO, "Properties::deinit()");
    //dataStr.clear();
    //dataInt.clear();
    fs.close();
}

int Properties::init_home_dir_prop(){
    char* _home_dir = getenv(HOME_ENV);
    if (!_home_dir || !strlen(_home_dir)){         
        struct passwd *pswd = getpwuid(getuid());        
        if (!pswd){
            LogMessage(ERROR, "Properties::init_home_dir_prop()::PROPERTY_ERROR_FILE_CANT_GET_USER_PSWD");
            return PROPERTY_ERROR_FILE_CANT_GET_USER_PSWD;
        }
        _home_dir = pswd->pw_dir;
        if (!strlen(_home_dir)){
            LogMessage(ERROR, "Properties::init_home_dir_prop()::PROPERTY_ERROR_FILE_CANT_GET_HOME_DIR");
            return PROPERTY_ERROR_FILE_CANT_GET_HOME_DIR;
        }
    }
    if(V_IS_ON(2)){
        LogMessage2(DEVELOP, "Properties::init_home_dir_prop() done, home_dir_path = ", _home_dir);
    }
    return set_anyway(KEY_HOME_DIR, _home_dir);
}

int Properties::add_key(const std::string& key, const std::string& value) {
    if (exist_string(key) == PROPERTY_NO_ERRORS){
        if(V_IS_ON(2)){
            LogMessage5(DEVELOP, "Can't add key in Propertiest::addkey( Key :", key.c_str(), " with value: ", value.c_str(),").  Key already exists");
        }
        return PROPERTY_ERROR_STR_KEY_ALREADY_EXISTS;
    }
    dataStr.insert({ key, value });
    if(V_IS_ON(2)){
        LogMessage5(DEVELOP, "Properties::add_key(",key.c_str(),", ", value.c_str(),") added successfully");
    }
    if (int writeDataReturn = update_strData_in_file(key))
        return writeDataReturn;
    if(V_IS_ON(2)){
        LogMessage5(DEVELOP, "Properties::add_key(",key.c_str(),", ", value.c_str(),") writed to file successfully");
    }
    return PROPERTY_NO_ERRORS;
}

int Properties::add_key(const std::string& key, const int value) {
    if (exist_int(key) == PROPERTY_NO_ERRORS){
        if(V_IS_ON(2)){
            LogMessage5(DEVELOP, "Can't add key in Propertiest::addkey( Key :", key.c_str(), " with value: ", std::to_string(value).c_str(),").  Key already exists");
        }
        return PROPERTY_ERROR_INT_KEY_ALREADY_EXISTS;
    }

    dataInt.insert({ key, value });
    if(V_IS_ON(2)){
        LogMessage5(DEVELOP, "Properties::add_key(",key.c_str(),", ", std::to_string(value).c_str(),") added successfully");
    }
    if (int writeDataReturn = update_strData_in_file(key)){
        if(V_IS_ON(1)){
            LogMessage6(INFO, "Properties::add_key(",key.c_str(),", ", 
                std::to_string(value).c_str(),")::writeDataReturn() has error message:", 
                std::to_string(writeDataReturn).c_str());
        }
        return writeDataReturn;
    }
    if(V_IS_ON(2)){
        LogMessage5(DEVELOP, "Properties::add_key(",key.c_str(),", ", std::to_string(value).c_str(),") writed to file successfully");
    }
    return PROPERTY_NO_ERRORS;
}

int Properties::set_value(const std::string& key, const std::string& value) {
    if(int existRet = exist_string(key)){
        return existRet;
    }

    dataStr[key] = value;
    if(V_IS_ON(2)){
        LogMessage5(DEVELOP, "Properties::set_value(",key.c_str(),", ", value.c_str(),") added successfully");
    }
    int writeDataReturn = update_strData_in_file(key);
    LogMessage6(DEVELOP, "Properties::set_value(",key.c_str(),", ", value.c_str(),") was writed to file with ret code = ", toStr(writeDataReturn).c_str());
    return writeDataReturn;
}


int Properties::set_anyway(const std::string& key, const int value){
    dataInt[key] = value;

    int writeDataReturn = update_intData_in_file(key);
    return writeDataReturn;
}

int Properties::set_anyway(const std::string& key, const std::string& value){
    dataStr[key] = value;

    int writeDataReturn = update_strData_in_file(key);
    return writeDataReturn;
}

int Properties::set_value(const std::string& key, const int value) { 
    if(int existRet = exist_int(key)){
        return existRet;
    }

    dataInt[key] = value;
    LogMessage5(DEVELOP, "Properties::set_value(",key.c_str(),", ", toStr(value).c_str(),") was added ");
    int writeDataReturn = update_intData_in_file(key);
    LogMessage6(DEVELOP, "Properties::set_value(",key.c_str(),", ", toStr(value).c_str(),") was writed to file with ret code = ", toStr(writeDataReturn).c_str());
    return writeDataReturn;
}

int Properties::get_value(const std::string& key, std::string& value){
    if(int existRet = exist_string(key)){
        LogMessage3(DEBUG, "Properties::get_value(", key.c_str(), ") not exists");
        return existRet;
    }
    value = dataStr.at(key);
    LogMessage5(DEVELOP, "Properties::get_value(",key.c_str(),") = ", value.c_str(),";");
    return PROPERTY_NO_ERRORS;    
}

int Properties::get_value(const std::string& key, int& value){
    if(int existRet = exist_int(key)){
        LogMessage3(DEVELOP, "Properties::get_value(", key.c_str(), ") not exists");
        return existRet;
    }
    
    value = dataInt.at(key);
    LogMessage5(DEVELOP, "Properties::get_value(",key.c_str(),", ", toStr(value).c_str(),")");
    return PROPERTY_NO_ERRORS;    
}

bool Properties::set_write_to_file(const bool writeToFile){
    
    LogMessage2(DEBUG, "set write to properties file ", std::to_string(writeToFile).c_str());
    fWriteToFile = writeToFile;
    if(fWriteToFile){
        write_all_data_to_file();
    }
    return fWriteToFile;
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
    return exist_string(key)==PROPERTY_NO_ERRORS || exist_int(key)==PROPERTY_NO_ERRORS;
}

int Properties::exist_int(const std::string& key){
    if (!exist_int_in_map(key)){ // if we don't hame key in memory-trying to load from file
        int updateRes = update_intData_from_file(key);
        if(updateRes != PROPERTY_NO_ERRORS){
            return updateRes;
        }
        if(!exist_int_in_map(key)){
            return PROPERTY_ERROR_INT_KEY_NOT_EXISTS;
        }
    }
    return PROPERTY_NO_ERRORS;
}
int Properties::exist_string(const std::string& key){
    if (!exist_string_in_map(key)){ // if we don't hame key in memory-trying to load from file
        int updateRes = update_strData_from_file(key);
        if(updateRes != PROPERTY_NO_ERRORS){
            return updateRes;
        }
        if(!exist_string_in_map(key)){
            return PROPERTY_ERROR_INT_KEY_NOT_EXISTS;
        }
    }
    return PROPERTY_NO_ERRORS;
}


int Properties::create_properties_file(){
//    LOG_DEVELOP_MSG << "Properties::create_properties_file()";
    LogMessage(DEVELOP, "Properties::create_properties_file()");
    fs.open(filename_str, std::ios::binary | std::ios::out | std::ofstream::trunc);
    fs.close();
    fs.open(filename_int, std::ios::binary | std::ios::out | std::ofstream::trunc);
    fs.close();
    return PROPERTY_NO_ERRORS;
}

int Properties::read_all_data_from_file() {
//    LOG_DEVELOP_MSG << "Properties::read_all_data_from_file()";
    LogMessage(DEVELOP, "Properties::read_all_data_from_file()");
    std::string line;
    std::string::size_type n;
    const char delim = '=';

    fs.open(filename_str, std::ios::binary | std::ios::in);
    if (!fs.is_open() || fs.eof()){
        if(CheckLogLevel(DEVELOP) && V_IS_ON(1)) 
            LogMessage2(WARNING, "Properties::read_all_data_from_file()::PROPERTY_ERROR_FILE_STRINGPROPERTIES_NOT_FOUND, path = ", filename_str.c_str() );
        return PROPERTY_ERROR_FILE_STRINGPROPERTIES_NOT_FOUND;
    }
    //dataStr.clear();
    //dataInt.clear();

    while (std::getline(fs, line)) {
        n = line.find(delim);
        if (n != std::string::npos)
            dataStr[ line.substr(0, n)] = line.substr(n + 1, std::string::npos) ;
    }
    fs.close();

    fs.open(filename_int, std::ios::binary | std::ios::in);

    if (!fs.is_open() || fs.eof()){
        if(V_IS_ON(2))
        return PROPERTY_ERROR_FILE_INTPROPERTIES_NOT_FOUND;
    }

    while (std::getline(fs, line)) {
        n = line.find(delim);
        if (n != std::string::npos){
            int num = std::stoi(line.substr(n + 1, std::string::npos));
            dataInt[line.substr(0, n)] = num ;
        }
    }
    fs.close();
    return PROPERTY_NO_ERRORS;
}


#define TURN_OFF_PROPERTIES_FILES
int Properties::write_all_data_to_file() {
    #ifndef TURN_OFF_PROPERTIES_FILES // 
    if(!fWriteToFile)
    #endif
    {
        if( V_IS_ON(1) ){
            LogMessage(DEBUG, "Properties::write_all_data_to_file()::PROPERTY_WRITING_TURNED_OFF");
        }
        return PROPERTY_WRITING_TURNED_OFF;
    }

    fs.open(filename_str, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!fs.is_open()){
        //if( V_IS_ON(1) )
        return PROPERTY_ERROR_FILE_CANT_OPEN;
    }

    for (auto it = dataStr.begin(); it != dataStr.end(); ++it)
        fs << it->first << "=" << it->second << "\n";
    fs.close();
    
    fs.open(filename_int, std::ios::binary | std::ios::out | std::ios::trunc);
    for (auto it = dataInt.begin(); it != dataInt.end(); ++it)
        fs << it->first << "=" << std::to_string(it->second) << "\n";
    fs.close();

    return PROPERTY_NO_ERRORS;
}

int Properties::update_intData_from_file(const std::string& key){
    std::string line;
    std::string search = key + "=";
    unsigned int curLine = 0;
    fs.open(filename_int, std::ios::binary | std::ios::in);
    while(getline(fs, line)) {
        curLine++;
        if (int pos = line.find(search, 0) != std::string::npos) {
            std::string value = line.substr(pos+1);
            int iVal = std::stoi(value);
            dataStr[key] = iVal;
            return PROPERTY_NO_ERRORS;
        }
    }

    if(V_IS_ON(2)){
       LogMessage3(DEVELOP, "Properties::update_intData_from_file( ", key.c_str(), " )::PROPERTY_ERROR_FILE_KEY_NOT_FOUND");
    }
    return PROPERTY_ERROR_FILE_KEY_NOT_FOUND;
}

int Properties::update_strData_from_file(const std::string& key){
    std::string line;
    std::string search = key + "=";
    unsigned int curLine = 0;
    fs.open(filename_str, std::ios::binary | std::ios::in);
    while(getline(fs, line)) {
        curLine++;
        if (int pos = line.find(search, 0) != std::string::npos) {
            std::string value = line.substr(pos+1);
            dataStr[key] = value;
            return PROPERTY_NO_ERRORS;
        }
    }
    if(V_IS_ON(2)){
        LogMessage3(DEVELOP, "Properties::update_strData_from_file( ", key.c_str(), " )::PROPERTY_ERROR_FILE_KEY_NOT_FOUND");
    }
    return PROPERTY_ERROR_FILE_KEY_NOT_FOUND;
}

int Properties::update_intData_in_file(const std::string& key){
    //TODO rewrite to update only needed data in file
    if(V_IS_ON(2)){
        LogMessage3(DEVELOP, "Properties::update_intData_in_file( ", key.c_str(), " )");
    }
    read_all_data_from_file();
    return write_all_data_to_file();
}
    
int Properties::update_strData_in_file(const std::string& key){
    if(V_IS_ON(2)){
        LogMessage3(DEVELOP, "Properties::update_strData_in_file( ", key.c_str(), " )");
    }
    read_all_data_from_file();
    return write_all_data_to_file();
}
