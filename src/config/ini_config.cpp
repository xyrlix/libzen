#include "config/ini_config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace zen {
namespace config {

bool ini_config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    std::string current_section = "";
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        if (line[0] == '[' && line.back() == ']') {
            current_section = trim(line.substr(1, line.size() - 2));
            continue;
        }
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = unescape(trim(line.substr(pos + 1)));
            
            if (current_section.empty()) {
                data_[""][key] = value;
            } else {
                data_[current_section][key] = value;
            }
        }
    }
    
    return true;
}

bool ini_config::save(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    for (const auto& section : data_) {
        if (!section.first.empty()) {
            file << "[" << section.first << "]" << std::endl;
        }
        
        for (const auto& kv : section.second) {
            file << kv.first << "=" << escape(kv.second) << std::endl;
        }
        
        file << std::endl;
    }
    
    return true;
}

std::string ini_config::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string ini_config::escape(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\n') {
            result += "\\n";
        } else if (c == '\r') {
            result += "\\r";
        } else if (c == '\t') {
            result += "\\t";
        } else if (c == '\\') {
            result += "\\\\";
        } else {
            result += c;
        }
    }
    return result;
}

std::string ini_config::unescape(const std::string& str) {
    std::string result;
    size_t i = 0;
    while (i < str.size()) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            char next = str[i + 1];
            if (next == 'n') {
                result += '\n';
                i += 2;
            } else if (next == 'r') {
                result += '\r';
                i += 2;
            } else if (next == 't') {
                result += '\t';
                i += 2;
            } else if (next == '\\') {
                result += '\\';
                i += 2;
            } else {
                result += str[i++];
            }
        } else {
            result += str[i++];
        }
    }
    return result;
}

config_value ini_config::get(const std::string& section, const std::string& key) const {
    auto sit = data_.find(section);
    if (sit == data_.end()) return config_value();
    
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return config_value();
    
    return config_value(kit->second);
}

void ini_config::set(const std::string& section, const std::string& key, const config_value& value) {
    data_[section][key] = value.as_string();
}

bool ini_config::has_section(const std::string& section) const {
    return data_.find(section) != data_.end();
}

bool ini_config::has_key(const std::string& section, const std::string& key) const {
    auto sit = data_.find(section);
    if (sit == data_.end()) return false;
    return sit->second.find(key) != sit->second.end();
}

bool ini_config::remove_section(const std::string& section) {
    return data_.erase(section) > 0;
}

bool ini_config::remove_key(const std::string& section, const std::string& key) {
    auto sit = data_.find(section);
    if (sit == data_.end()) return false;
    return sit->second.erase(key) > 0;
}

std::vector<std::string> ini_config::sections() const {
    std::vector<std::string> result;
    for (const auto& kv : data_) {
        result.push_back(kv.first);
    }
    return result;
}

std::vector<std::string> ini_config::keys(const std::string& section) const {
    std::vector<std::string> result;
    auto sit = data_.find(section);
    if (sit != data_.end()) {
        for (const auto& kv : sit->second) {
            result.push_back(kv.first);
        }
    }
    return result;
}

void ini_config::clear() {
    data_.clear();
}

} // namespace config
} // namespace zen
