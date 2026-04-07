#include <zen/filesystem.h>
#include <iostream>

using namespace zen;
using namespace zen::fs;

void path_example() {
    std::cout << "=== Path Examples ===" << std::endl;
    
    // 构造路径
    path p1("/home/user/document.txt");
    path p2("relative/path/to/file");
    
    std::cout << "Absolute path: " << p1.string() << std::endl;
    std::cout << "Relative path: " << p2.string() << std::endl;
    
    // 路径组件
    std::cout << "\nPath components:" << std::endl;
    std::cout << "  Filename: " << p1.filename() << std::endl;
    std::cout << "  Stem: " << p1.stem() << std::endl;
    std::cout << "  Extension: " << p1.extension() << std::endl;
    std::cout << "  Parent: " << p1.parent_path().string() << std::endl;
    
    // 路径拼接
    path p3 = "/home/user" / "documents" / "file.txt";
    std::cout << "\nJoined path: " << p3.string() << std::endl;
    
    // 路径规范化
    path p4 = "/usr/local/../bin/./script.sh";
    std::cout << "Original: " << p4.string() << std::endl;
    std::cout << "Normalized: " << p4.normalize().string() << std::endl;
    
    // 相对路径转换
    path abs = p3.absolute();
    std::cout << "\nAbsolute: " << abs.string() << std::endl;
    
    std::cout << std::endl;
}

void directory_example() {
    std::cout << "=== Directory Examples ===" << std::endl;
    
    path test_dir = "/tmp/libzen_fs_test";
    
    // 创建目录
    if (!exists(test_dir)) {
        if (create_directory(test_dir)) {
            std::cout << "Created directory: " << test_dir.string() << std::endl;
        }
    }
    
    // 创建嵌套目录
    path nested = test_dir / "a" / "b" / "c";
    if (create_directories(nested)) {
        std::cout << "Created nested directories: " << nested.string() << std::endl;
    }
    
    // 遍历目录
    std::cout << "\nDirectory listing:" << std::endl;
    directory_iterator it(test_dir);
    directory_iterator end;
    for (; it != end; ++it) {
        std::cout << "  " << it->path().string();
        if (it->is_directory()) {
            std::cout << " (DIR)";
        }
        std::cout << std::endl;
    }
    
    // 递归遍历
    std::cout << "\nRecursive listing:" << std::endl;
    recursive_directory_iterator rit(test_dir);
    recursive_directory_iterator rend;
    for (; rit != rend; ++rit) {
        std::string indent(rit.depth() * 2, ' ');
        std::cout << indent << rit->path().filename().string();
        if (rit->is_directory()) {
            std::cout << " (DIR)";
        }
        std::cout << std::endl;
    }
    
    // 获取当前工作目录
    path cwd = current_path();
    std::cout << "\nCurrent working directory: " << cwd.string() << std::endl;
    
    // 清理
    if (remove_all(test_dir)) {
        std::cout << "\nRemoved test directory" << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    path_example();
    directory_example();
    
    return 0;
}
