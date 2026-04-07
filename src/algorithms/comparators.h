#ifndef ZEN_ALGORITHMS_COMPARATORS_H
#define ZEN_ALGORITHMS_COMPARATORS_H

namespace zen {

// 比较器
template<typename T>
struct less {
    constexpr bool operator()(const T& a, const T& b) const noexcept {
        return a < b;
    }
};

} // namespace zen

#endif // ZEN_ALGORITHMS_COMPARATORS_H
