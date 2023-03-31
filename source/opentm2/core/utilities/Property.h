#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <string>
#include <fstream>
#include <map>
//linux defines
#define KEY_OTM_DIR "OTM_dir"
#define KEY_MEM_DIR "MEM_dir"
#define KEY_SERVICE_URL "KEY_SERVICE_URL"
#define KEY_HOME_DIR "HOME_dir"
#define KEY_ALLOWED_RAM "AllowedRAM_MB"
#define KEY_LOCALHOST_ONLY "localhost_only"
#define KEY_TEMP_DIR "TEMP_dir"
#define KEY_TRIPLES_THRESHOLD "TriplesThreshold"
#define KEY_NUM_OF_THREADS "KEY_NUM_OF_THREADS"
#define KEY_TIMEOUT_SETTINGS "KEY_TIMEOUT_SETTINGS"
#define KEY_RUN_DATE "KEY_RUN_DATE" 
//#define KEY_PLUGIN_DIR "PLUGIN_dir"
#define KEY_DEFAULT_DIR "DEFAULT_dir"
#define KEY_LANG_FILE_PATH "LANG_FILE_PATH"
#define KEY_DEBUG_KEYS_SEPARATE_FILES "WRITE_KEYS_IN_SEPARATE_FILES"

#define KEY_APP_VERSION "APP_VERSION"
//end def

enum PROPERTY_ERRORS {
    PROPERTY_NO_ERRORS = 0,
    PROPERTY_USED_DEFAULT_VALUE,
    PROPERTY_WRITING_TURNED_OFF,


    PROPERTY_ERROR_FILE_CANT_OPEN,
    PROPERTY_ERROR_FILE_STRINGPROPERTIES_NOT_FOUND,
    PROPERTY_ERROR_FILE_INTPROPERTIES_NOT_FOUND,

    PROPERTY_ERROR_FILE_CANT_CREATE_OTM_DIRECTORY,
    PROPERTY_ERROR_FILE_CANT_CREATE_PLUGIN_DIR,

    PROPERTY_ERROR_FILE_STR_KEY_NOT_FOUND,
    PROPERTY_ERROR_FILE_INT_KEY_NOT_FOUND,

    PROPERTY_ERROR_STR_KEY_NOT_EXISTS,
    PROPERTY_ERROR_STR_KEY_ALREADY_EXISTS,
    
    PROPERTY_ERROR_INT_KEY_NOT_EXISTS,
    PROPERTY_ERROR_INT_KEY_ALREADY_EXISTS,

    PROPERTY_ERROR_TEMPORARY_DISABLED_CODE,
    PROPERTY_ERROR_FILE_KEY_NOT_FOUND
};

class Properties {
public:    
    //checks if key exists in memory
    static Properties* GetInstance();
    bool exist(const std::string& key);

    int add_key(const std::string& key, const std::string& value);
    int set_value(const std::string& key, const std::string& value);
    int set_anyway(const std::string& key, const std::string& value);
    int get_value(const std::string& key, std::string& value);

    int add_key(const std::string& key, const int value);
    int set_value(const std::string& key, const int value);
    int set_anyway(const std::string& key, const int value);
    int get_value(const std::string& key, int& value);
    int get_value(const char* key, char *buff, int buffSize);
    int get_value_or_default(const char* key, char *buff, int buffSize, const char* defaultValue );
    int get_value_or_default(const char* key, int& value, const int defaultValue);

    bool set_write_to_file(const bool writeToFile);
private:

    //check memory and if not found- try to find in file
    int exist_int(const std::string& key);
    int exist_string(const std::string& key);

    std::map<std::string, std::string>  dataStr;
    std::map<std::string, int>          dataInt;
};

#endif // _PROPERTY_H_
