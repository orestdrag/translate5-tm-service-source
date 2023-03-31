#include "Property.h"

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

#define OTMMEMORYSERVICE "t5memory"

/* Implementation */


int Properties::add_key(const std::string& key, const std::string& value) {
    if (exist_string(key) == PROPERTY_NO_ERRORS){
        if(VLOG_IS_ON(2)){
            T5LOG( T5DEVELOP) <<  "Can't add key in Propertiest::addkey( Key :" << key << " with value: " << value << ").  Key already exists";
        }
        return PROPERTY_ERROR_STR_KEY_ALREADY_EXISTS;
    }
    dataStr.insert({ key, value });
    if(VLOG_IS_ON(2)){
        T5LOG( T5DEVELOP) << "Properties::add_key(" << key << ", " << value << ") added successfully";
    }
    return PROPERTY_NO_ERRORS;
}

int Properties::add_key(const std::string& key, const int value) {
    if (exist_int(key) == PROPERTY_NO_ERRORS){
        if(VLOG_IS_ON(2)){
            T5LOG( T5DEVELOP) << "Can't add key in Propertiest::addkey( Key :"<< key << " with value: " << value << ").  Key already exists";
        }
        return PROPERTY_ERROR_INT_KEY_ALREADY_EXISTS;
    }

    dataInt.insert({ key, value });
    if(VLOG_IS_ON(2)){
        T5LOG( T5DEVELOP) << "Properties::add_key(" << key <<", " << value << ") added successfully";
    }
    if(VLOG_IS_ON(2)){
        T5LOG( T5DEVELOP) << "Properties::add_key(" << key << ", " << value << ") writed to file successfully";
    }
    return PROPERTY_NO_ERRORS;
}

int Properties::set_value(const std::string& key, const std::string& value) {
    if(int existRet = exist_string(key)){
        return existRet;
    }
    dataStr[key] = value;
    if(VLOG_IS_ON(2)){
        T5LOG( T5DEVELOP) << "Properties::set_value(" << key << ", " << value << ") added successfully";
    }
}


int Properties::set_anyway(const std::string& key, const int value){
    dataInt[key] = value;
    return PROPERTY_NO_ERRORS;
}

int Properties::set_anyway(const std::string& key, const std::string& value){
    dataStr[key] = value;
    return PROPERTY_NO_ERRORS;
}

int Properties::set_value(const std::string& key, const int value) { 
    if(int existRet = exist_int(key)){
        return existRet;
    }

    dataInt[key] = value;
    T5LOG( T5DEVELOP) << "Properties::set_value(" << key << ", " << value << ") was added ";
}

int Properties::get_value(const std::string& key, std::string& value){
    if(int existRet = exist_string(key)){
        T5LOG( T5DEBUG) << "Properties::get_value(" << key << ") not exists";
        return existRet;
    }
    value = dataStr.at(key);
    T5LOG( T5DEVELOP) << "Properties::get_value(" << key << ") = " << value <<";";
    return PROPERTY_NO_ERRORS;    
}

int Properties::get_value(const char *key, char *buff, int buffSize ){
    std::string str;
    int res = get_value(key, str);
    
    if(res == PROPERTY_NO_ERRORS)
        strncpy(buff, str.c_str(),buffSize);
    return res;
}



int Properties::get_value_or_default(const char* key, char *buff, int buffSize, const char* defaultValue ){
    int res = get_value(key, buff, buffSize);
    if(res != PROPERTY_NO_ERRORS){
        if(buff != defaultValue){
            strncpy(buff, defaultValue, buffSize);
        }
        add_key(key, defaultValue);
        if(VLOG_IS_ON(2)){
            T5LOG( T5DEVELOP) << "Key " <<  key << " not saved yet -> saving default value : " << defaultValue;
        }
        return PROPERTY_USED_DEFAULT_VALUE;
    }
    return PROPERTY_NO_ERRORS;
}


int Properties::get_value_or_default(const char* key, int& value, const int defaultValue){
    int res = get_value(key, value);
    if(res != PROPERTY_NO_ERRORS){
        value = defaultValue;
        add_key(key, defaultValue);
        if(VLOG_IS_ON(1)){
            T5LOG( T5DEVELOP) << "Key " << key << " not saved yet -> saving default value : " << defaultValue;
        }
        return PROPERTY_USED_DEFAULT_VALUE;
    }
    return PROPERTY_NO_ERRORS;
}

int Properties::get_value(const std::string& key, int& value){
    if(int existRet = exist_int(key)){
        T5LOG( T5DEVELOP) << "Properties::get_value(" << key << ") not exists";
        return existRet;
    }
    
    value = dataInt.at(key);
    T5LOG( T5DEVELOP) << "Properties::get_value(" << key <<", " << value << ")";
    return PROPERTY_NO_ERRORS;    
}

bool Properties::exist(const std::string& key){
    return exist_string(key)==PROPERTY_NO_ERRORS || exist_int(key)==PROPERTY_NO_ERRORS;
}

int Properties::exist_int(const std::string& key){
    if (!dataInt.count(key)){ 
        return PROPERTY_ERROR_INT_KEY_NOT_EXISTS;
    }
    return PROPERTY_NO_ERRORS;
}
int Properties::exist_string(const std::string& key){
    if (!dataStr.count(key)){ 
        return PROPERTY_ERROR_INT_KEY_NOT_EXISTS;
    }
    return PROPERTY_NO_ERRORS;
}

Properties* Properties::GetInstance(){
    static Properties prop;
    return &prop;
}

