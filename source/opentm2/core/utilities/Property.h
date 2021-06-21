#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <string>
#include <fstream>
#include <map>

class Properties {
public:
    int init();
    void deinit();

    bool exist(const std::string& key);

    int add_key(const std::string& key, const std::string& value);
    int set_value(const std::string& key, const std::string& value);
    int get_value(const std::string& key, std::string& value);

    int add_key(const std::string& key, const int value);
    int set_value(const std::string& key, const int value);
    int get_value(const std::string& key, int& value);

    std::string get_otm_dir() const;

private:
    int get_home_dir();
    int create_otm_dir();
    int read_data();
    int write_data();

    bool existInt(const std::string& key);
    bool existString(const std::string& key);

    std::map<std::string, std::string>  dataStr;
    std::map<std::string, int>          dataInt;

    std::string filename;
    std::string otm_dir;
    std::string home_dir;

    std::fstream fs;
};

#endif // _PROPERTY_H_
