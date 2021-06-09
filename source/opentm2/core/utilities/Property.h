#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <string>
#include <fstream>
#include <map>

class Properties {
public:
    int init();
    void deinit();
    int add_key(const std::string& key, const std::string& value);
    int set_value(const std::string& key, const std::string& value);
    std::string get_otm_dir() const;

private:
    int get_home_dir();
    int create_otm_dir();
    int read_data();
    int write_data();

    std::map<std::string, std::string> data;

    std::string filename;
    std::string otm_dir;
    std::string home_dir;

    std::fstream fs;
};

#endif // _PROPERTY_H_
