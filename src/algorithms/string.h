/**
 * @file string.h
 * @brief 字符串算法模块
 * @details 提供字符串搜索和匹配算法
 *          包括 KMP、Boyer-Moore、Rabin-Karp、正则表达式等
 */

#ifndef ZEN_ALGORITHMS_STRING_H
#define ZEN_ALGORITHMS_STRING_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <utility>

namespace zen {

// ============================================================================
// KMP 算法 (Knuth-Morris-Pratt)
// ============================================================================

/**
 * @brief 构建 KMP 前缀函数（失败函数）
 * @param pattern 模式串
 * @return 前缀函数数组
 */
inline std::vector<size_t> kmp_build_prefix(const std::string& pattern) {
    size_t m = pattern.size();
    std::vector<size_t> prefix(m, 0);

    for (size_t i = 1, j = 0; i < m; ++i) {
        while (j > 0 && pattern[i] != pattern[j]) {
            j = prefix[j - 1];
        }
        if (pattern[i] == pattern[j]) {
            j++;
        }
        prefix[i] = j;
    }

    return prefix;
}

/**
 * @brief KMP 算法查找所有匹配位置
 * @param text 文本
 * @param pattern 模式串
 * @return 所有匹配位置的起始索引
 */
inline std::vector<size_t> kmp_search(const std::string& text, const std::string& pattern) {
    std::vector<size_t> positions;
    size_t n = text.size();
    size_t m = pattern.size();

    if (m == 0 || n < m) {
        return positions;
    }

    std::vector<size_t> prefix = kmp_build_prefix(pattern);

    for (size_t i = 0, j = 0; i < n; ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            j = prefix[j - 1];
        }
        if (text[i] == pattern[j]) {
            j++;
        }
        if (j == m) {
            positions.push_back(i - m + 1);
            j = prefix[j - 1];
        }
    }

    return positions;
}

/**
 * @brief KMP 算法查找第一个匹配位置
 */
inline size_t kmp_find_first(const std::string& text, const std::string& pattern) {
    auto positions = kmp_search(text, pattern);
    return positions.empty() ? std::string::npos : positions[0];
}

// ============================================================================
// Boyer-Moore 算法
// ============================================================================

/**
 * @brief 构建 Boyer-Moore 坏字符规则表
 */
inline std::vector<int> boyer_moore_build_bad_char(const std::string& pattern) {
    const int ALPHABET_SIZE = 256;
    std::vector<int> bad_char(ALPHABET_SIZE, -1);

    for (size_t i = 0; i < pattern.size(); ++i) {
        bad_char[static_cast<unsigned char>(pattern[i])] = static_cast<int>(i);
    }

    return bad_char;
}

/**
 * @brief Boyer-Moore 算法查找所有匹配位置
 */
inline std::vector<size_t> boyer_moore_search(const std::string& text, const std::string& pattern) {
    std::vector<size_t> positions;
    size_t n = text.size();
    size_t m = pattern.size();

    if (m == 0 || n < m) {
        return positions;
    }

    auto bad_char = boyer_moore_build_bad_char(pattern);

    for (size_t shift = 0; shift <= n - m; ) {
        size_t j = m - 1;

        while (j != static_cast<size_t>(-1) && pattern[j] == text[shift + j]) {
            j--;
        }

        if (j == static_cast<size_t>(-1)) {
            positions.push_back(shift);
            // 根据 bad_char 规则移动
            if (shift + m < n) {
                char next_char = text[shift + m];
                shift += m - bad_char[static_cast<unsigned char>(next_char)];
            } else {
                shift++;
            }
        } else {
            char mismatch_char = text[shift + j];
            if (bad_char[static_cast<unsigned char>(mismatch_char)] == -1) {
                shift += j + 1;
            } else {
                int bad_shift = bad_char[static_cast<unsigned char>(mismatch_char)];
                shift += std::max(1, static_cast<int>(j) - bad_shift);
            }
        }
    }

    return positions;
}

/**
 * @brief Boyer-Moore 算法查找第一个匹配位置
 */
inline size_t boyer_moore_find_first(const std::string& text, const std::string& pattern) {
    auto positions = boyer_moore_search(text, pattern);
    return positions.empty() ? std::string::npos : positions[0];
}

// ============================================================================
// Rabin-Karp 算法
// ============================================================================

/**
 * @brief Rabin-Karp 算法查找所有匹配位置
 * @param text 文本
 * @param pattern 模式串
 * @param base 基数（默认 256）
 * @param mod 模数（默认一个大质数）
 */
inline std::vector<size_t> rabin_karp_search(
    const std::string& text,
    const std::string& pattern,
    size_t base = 256,
    size_t mod = 101
) {
    std::vector<size_t> positions;
    size_t n = text.size();
    size_t m = pattern.size();

    if (m == 0 || n < m) {
        return positions;
    }

    size_t pattern_hash = 0;
    size_t text_hash = 0;
    size_t highest_power = 1;

    // 计算 base^(m-1) % mod
    for (size_t i = 0; i < m - 1; ++i) {
        highest_power = (highest_power * base) % mod;
    }

    // 计算初始哈希值
    for (size_t i = 0; i < m; ++i) {
        pattern_hash = (pattern_hash * base + pattern[i]) % mod;
        text_hash = (text_hash * base + text[i]) % mod;
    }

    // 滑动窗口
    for (size_t i = 0; i <= n - m; ++i) {
        if (pattern_hash == text_hash) {
            // 哈希匹配，验证实际字符串
            if (text.substr(i, m) == pattern) {
                positions.push_back(i);
            }
        }

        // 滚动哈希
        if (i < n - m) {
            text_hash = ((text_hash - text[i] * highest_power) * base + text[i + m]) % mod;
            if (text_hash < 0) {
                text_hash += mod;
            }
        }
    }

    return positions;
}

/**
 * @brief Rabin-Karp 算法查找第一个匹配位置
 */
inline size_t rabin_karp_find_first(
    const std::string& text,
    const std::string& pattern,
    size_t base = 256,
    size_t mod = 101
) {
    auto positions = rabin_karp_search(text, pattern, base, mod);
    return positions.empty() ? std::string::npos : positions[0];
}

// ============================================================================
// 字符串编辑距离 (Levenshtein Distance)
// ============================================================================

/**
 * @brief 计算两个字符串的编辑距离
 * @param str1 字符串 1
 * @param str2 字符串 2
 * @return 编辑距离（插入、删除、替换的最小操作数）
 */
inline size_t levenshtein_distance(const std::string& str1, const std::string& str2) {
    size_t m = str1.size();
    size_t n = str2.size();

    std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));

    // 初始化边界条件
    for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j) dp[0][j] = j;

    // 动态规划
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (str1[i - 1] == str2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({
                    dp[i - 1][j],     // 删除
                    dp[i][j - 1],     // 插入
                    dp[i - 1][j - 1]  // 替换
                });
            }
        }
    }

    return dp[m][n];
}

// ============================================================================
// 字符串处理工具
// ============================================================================

/**
 * @brief 分割字符串
 * @param str 输入字符串
 * @param delimiter 分隔符
 * @return 分割后的字符串列表
 */
inline std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(str);

    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return tokens;
}

/**
 * @brief 连接字符串
 * @param tokens 字符串列表
 * @param delimiter 分隔符
 * @return 连接后的字符串
 */
inline std::string join(const std::vector<std::string>& tokens, const std::string& delimiter) {
    if (tokens.empty()) {
        return "";
    }

    std::string result = tokens[0];
    for (size_t i = 1; i < tokens.size(); ++i) {
        result += delimiter + tokens[i];
    }

    return result;
}

/**
 * @brief 去除字符串首尾空白字符
 */
inline std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(str[start])) {
        start++;
    }

    size_t end = str.size();
    while (end > start && std::isspace(str[end - 1])) {
        end--;
    }

    return str.substr(start, end - start);
}

/**
 * @brief 去除字符串左侧空白字符
 */
inline std::string ltrim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(str[start])) {
        start++;
    }
    return str.substr(start);
}

/**
 * @brief 去除字符串右侧空白字符
 */
inline std::string rtrim(const std::string& str) {
    size_t end = str.size();
    while (end > 0 && std::isspace(str[end - 1])) {
        end--;
    }
    return str.substr(0, end);
}

/**
 * @brief 转换为小写
 */
inline std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * @brief 转换为大写
 */
inline std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

/**
 * @brief 检查字符串是否以指定前缀开头
 */
inline bool starts_with(const std::string& str, const std::string& prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }
    return str.compare(0, prefix.size(), prefix) == 0;
}

/**
 * @brief 检查字符串是否以指定后缀结尾
 */
inline bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) {
        return false;
    }
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * @brief 替换所有出现的子串
 * @param str 原字符串
 * @param from 要替换的子串
 * @param to 替换后的子串
 * @return 替换后的字符串
 */
inline std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    if (from.empty()) {
        return str;
    }

    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }

    return str;
}

/**
 * @brief 反转字符串
 */
inline std::string reverse(const std::string& str) {
    return std::string(str.rbegin(), str.rend());
}

/**
 * @brief 检查字符串是否为回文
 */
inline bool is_palindrome(const std::string& str) {
    size_t left = 0;
    size_t right = str.size() - 1;
    while (left < right) {
        if (str[left] != str[right]) {
            return false;
        }
        left++;
        right--;
    }
    return true;
}

// ============================================================================
// 字符串哈希
// ============================================================================

/**
 * @brief 计算字符串的哈希值（DJB2 算法）
 */
inline size_t hash_djb2(const std::string& str) {
    size_t hash = 5381;
    for (unsigned char c : str) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash;
}

/**
 * @brief 计算字符串的哈希值（SDBM 算法）
 */
inline size_t hash_sdbm(const std::string& str) {
    size_t hash = 0;
    for (unsigned char c : str) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

/**
 * @brief 计算字符串的哈希值（FNV-1a 算法）
 */
inline size_t hash_fnv1a(const std::string& str) {
    const size_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    const size_t FNV_PRIME = 1099511628211ULL;

    size_t hash = FNV_OFFSET_BASIS;
    for (unsigned char c : str) {
        hash ^= c;
        hash *= FNV_PRIME;
    }
    return hash;
}

// ============================================================================
// 字符串模式匹配（简单正则表达式支持）
// ============================================================================

/**
 * @brief 简单通配符匹配（支持 * 和 ?）
 * @param pattern 模式（* 匹配任意字符序列，? 匹配单个字符）
 * @param str 字符串
 * @return 是否匹配
 */
inline bool wildcard_match(const std::string& pattern, const std::string& str) {
    size_t m = pattern.size();
    size_t n = str.size();

    // dp[i][j] 表示 pattern[0:i] 是否匹配 str[0:j]
    std::vector<std::vector<bool>> dp(m + 1, std::vector<bool>(n + 1, false));

    // 空模式匹配空字符串
    dp[0][0] = true;

    // 处理模式开头的 *
    for (size_t i = 1; i <= m; ++i) {
        if (pattern[i - 1] == '*') {
            dp[i][0] = dp[i - 1][0];
        }
    }

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (pattern[i - 1] == '*') {
                // * 可以匹配 0 个或多个字符
                dp[i][j] = dp[i - 1][j] || dp[i][j - 1];
            } else if (pattern[i - 1] == '?' || pattern[i - 1] == str[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            }
        }
    }

    return dp[m][n];
}

/**
 * @brief 检查字符串是否只包含数字
 */
inline bool is_digits(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(),
        [](unsigned char c) { return std::isdigit(c); });
}

/**
 * @brief 检查字符串是否只包含字母
 */
inline bool is_alpha(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(),
        [](unsigned char c) { return std::isalpha(c); });
}

/**
 * @brief 检查字符串是否只包含字母或数字
 */
inline bool is_alnum(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(),
        [](unsigned char c) { return std::isalnum(c); });
}

}  // namespace zen

#endif  // ZEN_ALGORITHMS_STRING_H
