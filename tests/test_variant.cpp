/**
 * @file test_variant.cpp
 * @brief variant 模块测试
 */

#include <iostream>
#include <string>
#include <cassert>
#include "src/utility/variant.h"

int main() {
    std::cout << "=== libzen Variant Tests ===\n\n";
    
    // 测试1: 基本构造
    {
        std::cout << "Test 1: basic construction... ";
        zen::variant<int, double, std::string> v(42);
        if (v.index() == 0 && zen::get<int>(v) == 42) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试2: 拷贝构造
    {
        std::cout << "Test 2: copy construction... ";
        zen::variant<int, double, std::string> v1(42);
        zen::variant<int, double, std::string> v2(v1);
        if (v2.index() == 0 && zen::get<int>(v2) == 42) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试3: 移动构造
    {
        std::cout << "Test 3: move construction... ";
        zen::variant<int, double, std::string> v1(42);
        zen::variant<int, double, std::string> v2(std::move(v1));
        if (v2.index() == 0 && zen::get<int>(v2) == 42) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试4: 不同类型
    {
        std::cout << "Test 4: different types... ";
        zen::variant<int, double, std::string> v(3.14);
        if (v.index() == 1) {
            double val = zen::get<double>(v);
            if (val > 3.1 && val < 3.2) {
                std::cout << "PASS\n";
            } else {
                std::cout << "FAIL (value=" << val << ")\n";
                return 1;
            }
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试5: holds_alternative
    {
        std::cout << "Test 5: holds_alternative... ";
        zen::variant<int, double, std::string> v(42);
        if (v.holds_alternative<int>() && !v.holds_alternative<double>()) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试6: get_if
    {
        std::cout << "Test 6: get_if... ";
        zen::variant<int, double, std::string> v(42);
        int* pi = v.get_if<int>();
        double* pd = v.get_if<double>();
        if (pi != nullptr && pd == nullptr && *pi == 42) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试7: emplace
    {
        std::cout << "Test 7: emplace... ";
        zen::variant<int, double, std::string> v(42);
        v.emplace<std::string>("hello");
        if (v.index() == 2 && zen::get<std::string>(v) == "hello") {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试8: swap
    {
        std::cout << "Test 8: swap... ";
        zen::variant<int, double> v1(10);
        zen::variant<int, double> v2(20.5);
        v1.swap(v2);
        if (v1.index() == 1 && v2.index() == 0) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试9: comparison
    {
        std::cout << "Test 9: comparison... ";
        zen::variant<int, double> v1(42);
        zen::variant<int, double> v2(42);
        zen::variant<int, double> v3(10);
        if (v1 == v2 && v1 != v3) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    // 测试10: visit
    {
        std::cout << "Test 10: visit... ";
        zen::variant<int, double, std::string> v1(10);
        zen::variant<int, double, std::string> v2(3.14);
        zen::variant<int, double, std::string> v3(std::string("hello"));
        
        auto visitor = [](auto& val) -> std::string {
            using T = zen::remove_cvref_t<decltype(val)>;
            if constexpr (zen::is_same_v<T, int>) return "int";
            else if constexpr (zen::is_same_v<T, double>) return "double";
            else return "string";
        };
        
        std::string r1 = zen::visit(visitor, v1);
        std::string r2 = zen::visit(visitor, v2);
        std::string r3 = zen::visit(visitor, v3);
        
        if (r1 == "int" && r2 == "double" && r3 == "string") {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL (" << r1 << "," << r2 << "," << r3 << ")\n";
            return 1;
        }
    }
    
    // 测试11: Monostate
    {
        std::cout << "Test 11: Monostate... ";
        zen::variant<zen::Monostate> v;
        zen::variant<zen::Monostate> v2;
        if (v.index() == 0 && v == v2) {
            std::cout << "PASS\n";
        } else {
            std::cout << "FAIL\n";
            return 1;
        }
    }
    
    std::cout << "\n=== ALL TESTS PASSED ===\n";
    return 0;
}
