#include "filesystem/directory.h"
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>

namespace zen {
namespace fs {

bool create_directory(const path& p) {
    return mkdir(p.string().c_str(), 0755) == 0;
}

bool create_directories(const path& p) {
    if (p.exists()) return p.is_directory();
    
    path parent = p.parent_path();
    if (!parent.empty() && !parent.exists()) {
        if (!create_directories(parent)) {
            return false;
        }
    }
    
    return create_directory(p);
}

bool remove_directory(const path& p) {
    return rmdir(p.string().c_str()) == 0;
}

bool remove_all(const path& p) {
    if (!p.exists()) return false;
    
    if (p.is_file()) {
        return std::remove(p.string().c_str()) == 0;
    }
    
    if (p.is_directory()) {
        directory_iterator end;
        for (directory_iterator it(p); it != end; ++it) {
            if (!remove_all(it->path())) {
                return false;
            }
        }
        return remove_directory(p);
    }
    
    return false;
}

path current_path() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return path(buffer);
    }
    return path();
}

bool exists(const path& p) {
    struct stat st;
    return stat(p.string().c_str(), &st) == 0;
}

directory_entry::directory_entry(const path& p) : path_(p) {
    struct stat st;
    if (stat(p.string().c_str(), &st) == 0) {
        is_dir_ = S_ISDIR(st.st_mode);
        is_file_ = S_ISREG(st.st_mode);
    } else {
        is_dir_ = false;
        is_file_ = false;
    }
}

path directory_entry::path() const {
    return path_;
}

std::string directory_entry::filename() const {
    return path_.filename();
}

bool directory_entry::is_directory() const {
    return is_dir_;
}

bool directory_entry::is_file() const {
    return is_file_;
}

directory_iterator::directory_iterator() : dir_(nullptr) {
}

directory_iterator::directory_iterator(const path& p) {
    dir_ = opendir(p.string().c_str());
    if (dir_) {
        next();
    }
}

directory_iterator::~directory_iterator() {
    if (dir_) {
        closedir(dir_);
    }
}

directory_iterator::directory_iterator(directory_iterator&& other) 
    : dir_(other.dir_), entry_(other.entry_) {
    other.dir_ = nullptr;
}

directory_iterator& directory_iterator::operator=(directory_iterator&& other) {
    if (this != &other) {
        if (dir_) {
            closedir(dir_);
        }
        dir_ = other.dir_;
        entry_ = other.entry_;
        other.dir_ = nullptr;
    }
    return *this;
}

bool directory_iterator::operator==(const directory_iterator& other) const {
    return dir_ == other.dir_;
}

bool directory_iterator::operator!=(const directory_iterator& other) const {
    return !(*this == other);
}

directory_iterator& directory_iterator::operator++() {
    next();
    return *this;
}

directory_entry directory_iterator::operator*() const {
    return entry_;
}

directory_entry* directory_iterator::operator->() {
    return &entry_;
}

void directory_iterator::next() {
    struct dirent* ent;
    while ((ent = readdir(dir_)) != nullptr) {
        if (std::strcmp(ent->d_name, ".") == 0 || std::strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        entry_ = directory_entry(path(current_path()) / ent->d_name);
        return;
    }
    closedir(dir_);
    dir_ = nullptr;
}

recursive_directory_iterator::recursive_directory_iterator() : depth_(0) {
}

recursive_directory_iterator::recursive_directory_iterator(const path& p) 
    : current_(directory_iterator(p)), depth_(0) {
}

recursive_directory_iterator::recursive_directory_iterator(recursive_directory_iterator&& other)
    : current_(std::move(other.current_)), stack_(std::move(other.stack_)), depth_(other.depth_) {
}

recursive_directory_iterator& recursive_directory_iterator::operator++() {
    if (current_->is_directory()) {
        stack_.push_back(std::move(current_));
        current_ = directory_iterator(current_->path());
        ++depth_;
    } else {
        ++current_;
    }
    
    while (current_ == directory_iterator() && !stack_.empty()) {
        current_ = std::move(stack_.back());
        stack_.pop_back();
        --depth_;
        ++current_;
    }
    
    return *this;
}

directory_entry recursive_directory_iterator::operator*() const {
    return *current_;
}

directory_entry* recursive_directory_iterator::operator->() {
    return &(*current_);
}

bool recursive_directory_iterator::operator==(const recursive_directory_iterator& other) const {
    return current_ == other.current_;
}

bool recursive_directory_iterator::operator!=(const recursive_directory_iterator& other) const {
    return !(*this == other);
}

int recursive_directory_iterator::depth() const {
    return depth_;
}

} // namespace fs
} // namespace zen
