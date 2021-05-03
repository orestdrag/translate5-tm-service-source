#include <string>
#include <map>

class config {
public:
    config() {}
    config(const std::string& config_file) : filename(config_file) {}

    int parse();

    std::string get_value(const std::string& key, const std::string& def)
    { return data.count(key) ? data.at(key) : def; }

private:
    std::string filename = "../.OtmMemoryService.conf";

    std::map<std::string, std::string> data;
};
