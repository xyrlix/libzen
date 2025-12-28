#pragma once

#include <type_traits>
#include <stdexcept>
#include <limits>

namespace zen {

/**
 * @brief 安全类型转换工具类
 * 避免static_cast/reinterpret_cast的不安全问题
 */
class type_convert {
public:
    /**
     * @brief 安全的数值转换，带边界检查
     * @tparam To 目标类型
     * @tparam From 源类型
     * @param from 源值
     * @return 转换后的值
     * @throws std::overflow_error 如果转换会导致溢出
     */
    template <typename To, typename From>
    static To safe_numeric_cast(From from) {
        static_assert(std::is_arithmetic<To>::value, "To must be arithmetic type");
        static_assert(std::is_arithmetic<From>::value, "From must be arithmetic type");

        // 处理相同类型的转换
        if (std::is_same<To, From>::value) {
            return static_cast<To>(from);
        }

        // 处理源类型为布尔值的情况
        if (std::is_same<From, bool>::value) {
            return static_cast<To>(from ? 1 : 0);
        }

        // 处理目标类型为布尔值的情况
        if (std::is_same<To, bool>::value) {
            return static_cast<To>(from != 0);
        }

        // 处理有符号到无符号的转换
        if (std::is_signed<From>::value && !std::is_signed<To>::value) {
            if (from < 0 || static_cast<typename std::make_unsigned<From>::type>(from) > std::numeric_limits<To>::max()) {
                throw std::overflow_error("safe_numeric_cast: overflow");
            }
        }
        // 处理无符号到有符号的转换
        else if (!std::is_signed<From>::value && std::is_signed<To>::value) {
            if (from > static_cast<typename std::make_unsigned<To>::type>(std::numeric_limits<To>::max())) {
                throw std::overflow_error("safe_numeric_cast: overflow");
            }
        }
        // 处理有符号到有符号的转换
        else if (std::is_signed<From>::value && std::is_signed<To>::value) {
            if (from < std::numeric_limits<To>::min() || from > std::numeric_limits<To>::max()) {
                throw std::overflow_error("safe_numeric_cast: overflow");
            }
        }
        // 处理无符号到无符号的转换
        else {
            if (from > std::numeric_limits<To>::max()) {
                throw std::overflow_error("safe_numeric_cast: overflow");
            }
        }

        return static_cast<To>(from);
    }

    /**
     * @brief 安全的向下转型，带类型检查
     * @tparam To 目标类型
     * @tparam From 源类型
     * @param from 源指针
     * @return 转换后的值
     * @throws std::bad_cast 如果转换不安全
     */
    template <typename To, typename From>
    static To* safe_downcast(From* from) {
        static_assert(std::is_polymorphic<From>::value, "From must be polymorphic type for safe_downcast");
        
        To* result = dynamic_cast<To*>(from);
        if (!result) {
            throw std::bad_cast();
        }
        return result;
    }

    /**
     * @brief 安全的向下转型，带类型检查（常量版本）
     * @tparam To 目标类型
     * @tparam From 源类型
     * @param from 源指针
     * @return 转换后的值
     * @throws std::bad_cast 如果转换不安全
     */
    template <typename To, typename From>
    static const To* safe_downcast(const From* from) {
        static_assert(std::is_polymorphic<From>::value, "From must be polymorphic type for safe_downcast");
        
        const To* result = dynamic_cast<const To*>(from);
        if (!result) {
            throw std::bad_cast();
        }
        return result;
    }

    /**
     * @brief 安全的向下转型，返回nullptr而不是抛出异常
     * @tparam To 目标类型
     * @tparam From 源类型
     * @param from 源指针
     * @return 转换后的值，如果转换不安全则返回nullptr
     */
    template <typename To, typename From>
    static To* safe_downcast_or_null(From* from) {
        static_assert(std::is_polymorphic<From>::value, "From must be polymorphic type for safe_downcast_or_null");
        
        return dynamic_cast<To*>(from);
    }

    /**
     * @brief 安全的向下转型，返回nullptr而不是抛出异常（常量版本）
     * @tparam To 目标类型
     * @tparam From 源类型
     * @param from 源指针
     * @return 转换后的值，如果转换不安全则返回nullptr
     */
    template <typename To, typename From>
    static const To* safe_downcast_or_null(const From* from) {
        static_assert(std::is_polymorphic<From>::value, "From must be polymorphic type for safe_downcast_or_null");
        
        return dynamic_cast<const To*>(from);
    }

    /**
     * @brief 字符串转整数，带边界检查
     * @tparam T 整数类型
     * @param str 字符串
     * @return 转换后的整数
     * @throws std::invalid_argument 如果字符串格式不正确
     * @throws std::overflow_error 如果转换会导致溢出
     */
    template <typename T>
    static T string_to_int(const std::string& str) {
        static_assert(std::is_integral<T>::value, "T must be integral type");
        
        if (str.empty()) {
            throw std::invalid_argument("string_to_int: empty string");
        }

        size_t pos = 0;
        long long result = 0;
        bool negative = false;

        // 处理符号
        if (str[0] == '-') {
            negative = true;
            pos = 1;
        } else if (str[0] == '+') {
            pos = 1;
        }

        // 处理每一位数字
        for (; pos < str.size(); ++pos) {
            char c = str[pos];
            if (c < '0' || c > '9') {
                throw std::invalid_argument("string_to_int: invalid character");
            }

            int digit = c - '0';
            
            // 检查溢出
            if (result > (std::numeric_limits<long long>::max() - digit) / 10) {
                throw std::overflow_error("string_to_int: overflow");
            }
            
            result = result * 10 + digit;
        }

        // 处理负数
        if (negative) {
            result = -result;
        }

        // 转换到目标类型并检查溢出
        return safe_numeric_cast<T>(result);
    }

    /**
     * @brief 字符串转浮点数
     * @tparam T 浮点数类型
     * @param str 字符串
     * @return 转换后的浮点数
     * @throws std::invalid_argument 如果字符串格式不正确
     */
    template <typename T>
    static T string_to_float(const std::string& str) {
        static_assert(std::is_floating_point<T>::value, "T must be floating point type");
        
        if (str.empty()) {
            throw std::invalid_argument("string_to_float: empty string");
        }

        size_t pos = 0;
        T result = 0;
        bool negative = false;

        // 处理符号
        if (str[0] == '-') {
            negative = true;
            pos = 1;
        } else if (str[0] == '+') {
            pos = 1;
        }

        // 处理整数部分
        while (pos < str.size() && str[pos] >= '0' && str[pos] <= '9') {
            result = result * 10 + (str[pos] - '0');
            ++pos;
        }

        // 处理小数部分
        if (pos < str.size() && str[pos] == '.') {
            ++pos;
            T fraction = 0.1;
            while (pos < str.size() && str[pos] >= '0' && str[pos] <= '9') {
                result += (str[pos] - '0') * fraction;
                fraction *= 0.1;
                ++pos;
            }
        }

        // 处理指数部分
        if (pos < str.size() && (str[pos] == 'e' || str[pos] == 'E')) {
            ++pos;
            bool exp_negative = false;
            int exponent = 0;

            if (pos < str.size() && (str[pos] == '-' || str[pos] == '+')) {
                exp_negative = (str[pos] == '-');
                ++pos;
            }

            while (pos < str.size() && str[pos] >= '0' && str[pos] <= '9') {
                exponent = exponent * 10 + (str[pos] - '0');
                ++pos;
            }

            if (exp_negative) {
                exponent = -exponent;
            }

            // 计算指数
            while (exponent > 0) {
                result *= 10;
                --exponent;
            }
            while (exponent < 0) {
                result /= 10;
                ++exponent;
            }
        }

        // 检查是否还有剩余字符
        if (pos < str.size()) {
            throw std::invalid_argument("string_to_float: invalid character");
        }

        // 处理负数
        if (negative) {
            result = -result;
        }

        return result;
    }

    /**
     * @brief 整数转字符串
     * @tparam T 整数类型
     * @param value 整数
     * @return 转换后的字符串
     */
    template <typename T>
    static std::string int_to_string(T value) {
        static_assert(std::is_integral<T>::value, "T must be integral type");
        
        if (value == 0) {
            return "0";
        }

        std::string result;
        bool negative = false;

        // 处理负数
        if (std::is_signed<T>::value && value < 0) {
            negative = true;
            value = -value;
        }

        // 转换每一位数字
        while (value > 0) {
            result.insert(result.begin(), '0' + (value % 10));
            value /= 10;
        }

        // 添加负号
        if (negative) {
            result.insert(result.begin(), '-');
        }

        return result;
    }

    /**
     * @brief 浮点数转字符串
     * @tparam T 浮点数类型
     * @param value 浮点数
     * @param precision 小数位数
     * @return 转换后的字符串
     */
    template <typename T>
    static std::string float_to_string(T value, int precision = 6) {
        static_assert(std::is_floating_point<T>::value, "T must be floating point type");
        
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%%.%df", precision);
        
        char result[64];
        snprintf(result, sizeof(result), buffer, value);
        
        return std::string(result);
    }
};

} // namespace zen
