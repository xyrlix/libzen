#include "filesystem/path.h"
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

namespace zen {
namespace fs {

path::path(const std::string& p) : path_(p) {
    normalize();
}

path& path::operator/=(const path& p) {
    if (path_.empty() || path_.back() != '/') {
        path_ += '/';
    }
    path_ += p.path_;
    normalize();
    return *this;
}

path path::operator/(const path& p) const {
    path result = *this;
    result /= p;
    return result;
}

path& path::append(const std::string& p) {
    return *this /= path(p);
}

std::string path::string() const {
    return path_;
}

std::string path::filename() const {
    size_t pos = path_.find_last_of('/');
    if (pos == std::string::npos) return path_;
    return path_.substr(pos + 1);
}

std::string path::stem() const {
    std::string fname = filename();
    size_t pos = fname.find_last_of('.');
    if (pos == std::string::npos || pos == 0) return fname;
    return fname.substr(0, pos);
}

std::string path::extension() const {
    std::string fname = filename();
    size_t pos = fname.find_last_of('.');
    if (pos == std::string::npos || pos == 0) return "";
    return fname.substr(pos);
}

path path::parent_path() const {
    size_t pos = path_.find_last_of('/');
    if (pos == std::string::npos) return path();
    return path(path_.substr(0, pos));
}

bool path::is_absolute() const {
    return !path_.empty() && path_[0] == '/';
}

bool path::is_relative() const {
    return !is_absolute();
}

void path::normalize() {
    std::vector<std::string> parts;
    std::string part;
    
    for (char c : path_) {
        if (c == '/') {
            if (!part.empty()) {
                if (part == "..") {
                    if (!parts.empty() && parts.back() != "..") {
                        parts.pop_back();
                    } else {
                        parts.push_back(part);
                    }
                } else if (part != ".") {
                    parts.push_back(part);
                }
                part.clear();
            }
        } else {
            part += c;
        }
    }
    
    if (!part.empty()) {
        if (part == "..") {
            if (!parts.empty() && parts.back() != "..") {
                parts.pop_back();
            } else {
                parts.push_back(part);
            }
        } else if (part != ".") {
            parts.push_back(part);
        }
    }
    
    if (parts.empty()) {
        path_ = "/";
    } else {
        path_.clear();
        for (const auto& p : parts) {
            path_ += "/" + p;
        }
    }
}

bool path::exists() const {
    struct stat st;
    return stat(path_.c_str(), &st) == 0;
}

bool path::is_directory() const {
    struct stat st;
    return stat(path_.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool path::is_file() const {
    struct stat st;
    return stat(path_.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

path path::absolute() const {
    if (is_absolute()) return *this;
    
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) == nullptr) {
        return *this;
    }
    return path(std::string(buffer)) / *this;
}

path path::relative(const path& base) const {
    std::string abs_this = this->absolute().string();
    std::string abs_base = base.absolute().string();
    
    if (abs_this.find(abs_base) != 0) {
        return *this;
    }
    
    return path(abs_this.substr(abs_base.length()));
}

std::string path::native() const {
    return string();
}

bool operator==(const path& lhs, const path& rhs) {
    return lhs.string() == rhs.string();
}

bool operator!=(const path& lhs, const path& rhs) {
    return !(lhs == rhs);
}

path operator/(const path& lhs, const path& rhs) {
    return lhs / rhs;
}

} // namespace fs
} // namespace zen
