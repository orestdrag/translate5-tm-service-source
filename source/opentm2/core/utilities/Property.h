#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <string>
#include <fstream>
#include <map>

class Properties {
public:
    int init();
    void deinit();
    
    //checks if key exists in memory
    bool existInMap(const std::string& key);

    int add_key(const std::string& key, const std::string& value);
    int set_value(const std::string& key, const std::string& value);
    int get_value(const std::string& key, std::string& value);

    int add_key(const std::string& key, const int value);
    int set_value(const std::string& key, const int value);
    int get_value(const std::string& key, int& value);

    std::string get_otm_dir() const;

    enum ERRORS {
        NO_ERRORS = 0,

        ERROR_FILE_CANT_OPEN,
        ERROR_FILE_STRINGPROPERTIES_NOT_FOUND,
        ERROR_FILE_INTPROPERTIES_NOT_FOUND,
        ERROR_FILE_CANT_CREATE_OTM_DIRECTORY,
        ERROR_FILE_CANT_CREATE_FILE,
        ERROR_FILE_CANT_GET_HOME_DIR,
        ERROR_FILE_CANT_GET_USER_PSWD,
        ERROR_FILE_KEY_NOT_FOUND,

        ERROR_STR_KEY_NOT_EXISTS,
        ERROR_STR_KEY_ALREADY_EXISTS,
        
        ERROR_INT_KEY_NOT_EXISTS,
        ERROR_INT_KEY_ALREADY_EXISTS

        
    };

private:
    int get_home_dir();
    int create_otm_dir();
    int create_properties_file();

    int update_intData_from_file(const std::string& key);
    int update_strData_from_file(const std::string& key);

    int update_intData_in_file(const std::string& key);
    int update_strData_in_file(const std::string& key);

    int read_all_data_from_file();
    int write_all_data_to_file();

    bool existIntInMap(const std::string& key);
    bool existStringInMap(const std::string& key);

    std::map<std::string, std::string>  dataStr;
    std::map<std::string, int>          dataInt;

    std::string filename;
    std::string otm_dir;
    std::string home_dir;

    std::fstream fs;
};

#endif // _PROPERTY_H_
