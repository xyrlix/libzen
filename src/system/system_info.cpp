#include "system/system_info.h"
#include <unistd.h>
#include <sys/sysinfo.h>
#include <cstring>

namespace zen {
namespace system {

int cpu_count() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

uint64_t total_memory() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.totalram * info.mem_unit;
    }
    return 0;
}

uint64_t available_memory() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.freeram * info.mem_unit;
    }
    return 0;
}

std::string hostname() {
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        return std::string(buffer);
    }
    return "";
}

std::string username() {
    char buffer[256];
    if (getlogin_r(buffer, sizeof(buffer)) == 0) {
        return std::string(buffer);
    }
    return "";
}

int pid() {
    return getpid();
}

int ppid() {
    return getppid();
}

int tid() {
    return gettid();
}

std::string executable_path() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer);
    }
    return "";
}

std::string current_working_directory() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    }
    return "";
}

uint64_t boot_time() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.uptime;
    }
    return 0;
}

} // namespace system
} // namespace zen
