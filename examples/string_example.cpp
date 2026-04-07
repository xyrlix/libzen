/**
 * @file string_example.cpp
 * @brief 字符串算法使用示例
 */

#include <iostream>
#include "algorithms/string.h"

using namespace zen;

void example_string_search() {
    std::cout << "=== 字符串搜索示例 ===\n";

    std::string text = "ABABDABACDABABCABAB";
    std::string pattern = "ABABCABAB";

    // KMP 搜索
    auto kmp_pos = kmp_search(text, pattern);
    std::cout << "KMP 搜索结果: ";
    for (auto pos : kmp_pos) {
        std::cout << pos << " ";
    }
    std::cout << "\n";

    // Boyer-Moore 搜索
    auto bm_pos = boyer_moore_search(text, pattern);
    std::cout << "Boyer-Moore 搜索结果: ";
    for (auto pos : bm_pos) {
        std::cout << pos << " ";
    }
    std::cout << "\n";

    // Rabin-Karp 搜索
    auto rk_pos = rabin_karp_search(text, pattern);
    std::cout << "Rabin-Karp 搜索结果: ";
    for (auto pos : rk_pos) {
        std::cout << pos << " ";
    }
    std::cout << "\n\n";
}

void example_string_edit_distance() {
    std::cout << "=== 编辑距离示例 ===\n";

    std::string str1 = "kitten";
    std::string str2 = "sitting";

    size_t dist = levenshtein_distance(str1, str2);
    std::cout << "编辑距离 \"" << str1 << "\" -> \"" << str2 << "\": " << dist << "\n";
    std::cout << "说明: 需要将 " << str1 << " 转换为 " << str2 << "\n";
    std::cout << "  操作:\n";
    std::cout << "    k -> s (替换)\n";
    std::cout << "    e -> i (替换)\n";
    std::cout << "    插入 g\n";
    std::cout << "\n";
}

void example_string_tools() {
    std::cout << "=== 字符串工具示例 ===\n";

    // 分割字符串
    std::string csv = "apple,banana,cherry,date";
    auto tokens = split(csv, ',');
    std::cout << "分割结果: ";
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << "\n";

    // 连接字符串
    std::vector<std::string> words = {"Hello", "World", "C++"};
    std::string joined = join(words, " ");
    std::cout << "连接结果: " << joined << "\n";

    // 去除空白
    std::string s = "  hello world  ";
    std::cout << "原始: \"" << s << "\"\n";
    std::cout << "trim: \"" << trim(s) << "\"\n";
    std::cout << "ltrim: \"" << ltrim(s) << "\"\n";
    std::cout << "rtrim: \"" << rtrim(s) << "\"\n";

    // 大小写转换
    std::cout << "to_lower: \"" << to_lower("HELLO") << "\"\n";
    std::cout << "to_upper: \"" << to_upper("hello") << "\"\n";

    // 前缀和后缀检查
    std::cout << "starts_with(\"hello world\", \"hello\"): "
              << (starts_with("hello world", "hello") ? "true" : "false") << "\n";
    std::cout << "ends_with(\"hello world\", \"world\"): "
              << (ends_with("hello world", "world") ? "true" : "false") << "\n";

    // 替换
    std::cout << "replace_all(\"hello world\", \"world\", \"there\"): "
              << replace_all("hello world", "world", "there") << "\n";

    // 反转
    std::cout << "reverse(\"hello\"): \"" << reverse("hello") << "\"\n";

    // 回文检查
    std::cout << "is_palindrome(\"racecar\"): "
              << (is_palindrome("racecar") ? "true" : "false") << "\n";
    std::cout << "is_palindrome(\"hello\"): "
              << (is_palindrome("hello") ? "true" : "false") << "\n";

    std::cout << "\n";
}

void example_string_hash() {
    std::cout << "=== 字符串哈希示例 ===\n";

    std::string s = "hello world";

    std::cout << "DJB2 哈希: " << hash_djb2(s) << "\n";
    std::cout << "SDBM 哈希: " << hash_sdbm(s) << "\n";
    std::cout << "FNV-1a 哈希: " << hash_fnv1a(s) << "\n";

    // 相同字符串的哈希值应该相同
    std::cout << "DJB2(\"hello\") == DJB2(\"hello\"): "
              << (hash_djb2("hello") == hash_djb2("hello") ? "true" : "false") << "\n";

    // 不同字符串的哈希值应该不同
    std::cout << "DJB2(\"hello\") == DJB2(\"world\"): "
              << (hash_djb2("hello") == hash_djb2("world") ? "true" : "false") << "\n";

    std::cout << "\n";
}

void example_wildcard_match() {
    std::cout << "=== 通配符匹配示例 ===\n";

    std::cout << "wildcard_match(\"hello\", \"h*\"): "
              << (wildcard_match("hello", "h*") ? "匹配" : "不匹配") << "\n";

    std::cout << "wildcard_match(\"hello\", \"h?llo\"): "
              << (wildcard_match("hello", "h?llo") ? "匹配" : "不匹配") << "\n";

    std::cout << "wildcard_match(\"hello\", \"*o\"): "
              << (wildcard_match("hello", "*o") ? "匹配" : "不匹配") << "\n";

    std::cout << "wildcard_match(\"hello\", \"a*\"): "
              << (wildcard_match("hello", "a*") ? "匹配" : "不匹配") << "\n";

    std::cout << "wildcard_match(\"hello\", \"h*l*o\"): "
              << (wildcard_match("hello", "h*l*o") ? "匹配" : "不匹配") << "\n";

    std::cout << "\n";
}

void example_string_validation() {
    std::cout << "=== 字符串验证示例 ===\n";

    std::cout << "is_digits(\"12345\"): "
              << (is_digits("12345") ? "true" : "false") << "\n";

    std::cout << "is_digits(\"12a45\"): "
              << (is_digits("12a45") ? "true" : "false") << "\n";

    std::cout << "is_alpha(\"hello\"): "
              << (is_alpha("hello") ? "true" : "false") << "\n";

    std::cout << "is_alpha(\"hello123\"): "
              << (is_alpha("hello123") ? "true" : "false") << "\n";

    std::cout << "is_alnum(\"hello123\"): "
              << (is_alnum("hello123") ? "true" : "false") << "\n";

    std::cout << "is_alnum(\"hello 123\"): "
              << (is_alnum("hello 123") ? "true" : "false") << "\n";

    std::cout << "\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "    字符串算法示例\n";
    std::cout << "========================================\n\n";

    example_string_search();
    example_string_edit_distance();
    example_string_tools();
    example_string_hash();
    example_wildcard_match();
    example_string_validation();

    return 0;
}
