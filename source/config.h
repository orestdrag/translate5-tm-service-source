#include <string>
#include <map>
#include "./opentm2/core/utilities/LogWrapper.h"
class config {
public:
    config() {}
    config(const std::string& config_file) : filename(config_file) {}

    int parse();

    std::string get_value(const std::string& key, const std::string& def)
    { 
        bool exists =  data.count(key) ;
        if(exists){
            return data.at(key);
        }else{ 
            LogMessage8(WARNING, __func__,":: key \'", key.c_str() ,"\'not found in file \'", filename.c_str(), "\', used default value instead def=\'", def.c_str(),"\'");
            return def;
        } 
    }

private:
    std::string filename = "~/.t5memory/.t5memory.conf";

    std::map<std::string, std::string> data;
};
