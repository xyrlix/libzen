#ifndef ZEN_CONTAINERS_ASSOCIATIVE_UNORDERED_MAP_H
#define ZEN_CONTAINERS_ASSOCIATIVE_UNORDERED_MAP_H

#include "../../base/type_traits.h"
#include <cstdint>  // uintptr_t
#include "../../utility/swap.h"
#include "../../utility/pair.h"
#include "../../memory/allocator.h"
#include "../../iterators/iterator_base.h"

namespace zen {

// ============================================================================
// 哈希函数（FNV-1a，不依赖 std::hash）
// ============================================================================

template<typename Key>
struct hash;

template<typename T>
struct integral_hash {
    size_t operator()(T v) const noexcept {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&v);
        size_t h = static_cast<size_t>(14695981039346656037ULL);
        for (size_t i = 0; i < sizeof(T); ++i) {
            h ^= static_cast<size_t>(p[i]);
            h *= static_cast<size_t>(1099511628211ULL);
        }
        return h;
    }
};

template<> struct hash<bool>               : integral_hash<bool>               {};
template<> struct hash<char>               : integral_hash<char>               {};
template<> struct hash<signed char>        : integral_hash<signed char>        {};
template<> struct hash<unsigned char>      : integral_hash<unsigned char>      {};
template<> struct hash<short>              : integral_hash<short>              {};
template<> struct hash<unsigned short>     : integral_hash<unsigned short>     {};
template<> struct hash<int>                : integral_hash<int>                {};
template<> struct hash<unsigned int>       : integral_hash<unsigned int>       {};
template<> struct hash<long>               : integral_hash<long>               {};
template<> struct hash<unsigned long>      : integral_hash<unsigned long>      {};
template<> struct hash<long long>          : integral_hash<long long>          {};
template<> struct hash<unsigned long long> : integral_hash<unsigned long long> {};
template<> struct hash<float>              : integral_hash<float>              {};
template<> struct hash<double>             : integral_hash<double>             {};

template<typename T>
struct hash<T*> {
    size_t operator()(T* p) const noexcept {
        return integral_hash<uintptr_t>{}(reinterpret_cast<uintptr_t>(p));
    }
};

// ============================================================================
// 键相等比较器
// ============================================================================

template<typename T>
struct equal_to {
    constexpr bool operator()(const T& a, const T& b) const noexcept {
        return a == b;
    }
};

// ============================================================================
// 哈希桶节点
// ============================================================================

template<typename Key, typename Value>
struct hash_node {
    using value_type = pair<const Key, Value>;

    value_type   kv;
    hash_node*   next;
    size_t       hash_val;

    template<typename... Args>
    explicit hash_node(size_t hv, Args&&... args)
        : kv(static_cast<Args&&>(args)...), next(nullptr), hash_val(hv) {}
};

// ============================================================================
// 迭代器
// ============================================================================

template<typename Key, typename Value>
struct umap_iterator {
    using node_type         = hash_node<Key, Value>;
    using value_type        = pair<const Key, Value>;
    using reference         = value_type&;
    using pointer           = value_type*;
    using difference_type   = decltype((char*)0 - (char*)0);
    using iterator_category = forward_iterator_tag;

    node_type** buckets_;
    size_t      bucket_count_;
    size_t      bucket_idx_;
    node_type*  node_;

    umap_iterator() noexcept : buckets_(nullptr), bucket_count_(0), bucket_idx_(0), node_(nullptr) {}
    umap_iterator(node_type** b, size_t bc, size_t bi, node_type* n) noexcept
        : buckets_(b), bucket_count_(bc), bucket_idx_(bi), node_(n) {}

    reference operator*()  const noexcept { return node_->kv; }
    pointer   operator->() const noexcept { return &node_->kv; }

    umap_iterator& operator++() noexcept {
        node_ = node_->next;
        if (!node_) {
            ++bucket_idx_;
            while (bucket_idx_ < bucket_count_ && !buckets_[bucket_idx_]) ++bucket_idx_;
            node_ = (bucket_idx_ < bucket_count_) ? buckets_[bucket_idx_] : nullptr;
        }
        return *this;
    }

    umap_iterator operator++(int) noexcept { umap_iterator t = *this; ++(*this); return t; }
    bool operator==(const umap_iterator& o) const noexcept { return node_ == o.node_; }
    bool operator!=(const umap_iterator& o) const noexcept { return node_ != o.node_; }
};

template<typename Key, typename Value>
struct umap_const_iterator {
    using node_type         = hash_node<Key, Value>;
    using value_type        = const pair<const Key, Value>;
    using reference         = const pair<const Key, Value>&;
    using pointer           = const pair<const Key, Value>*;
    using difference_type   = decltype((char*)0 - (char*)0);
    using iterator_category = forward_iterator_tag;

    const node_type** buckets_;
    size_t            bucket_count_;
    size_t            bucket_idx_;
    const node_type*  node_;

    umap_const_iterator() noexcept : buckets_(nullptr), bucket_count_(0), bucket_idx_(0), node_(nullptr) {}
    umap_const_iterator(const node_type** b, size_t bc, size_t bi, const node_type* n) noexcept
        : buckets_(b), bucket_count_(bc), bucket_idx_(bi), node_(n) {}
    umap_const_iterator(const umap_iterator<Key, Value>& it) noexcept
        : buckets_(const_cast<const node_type**>(it.buckets_)),
          bucket_count_(it.bucket_count_), bucket_idx_(it.bucket_idx_), node_(it.node_) {}

    reference operator*()  const noexcept { return node_->kv; }
    pointer   operator->() const noexcept { return &node_->kv; }

    umap_const_iterator& operator++() noexcept {
        node_ = node_->next;
        if (!node_) {
            ++bucket_idx_;
            while (bucket_idx_ < bucket_count_ && !buckets_[bucket_idx_]) ++bucket_idx_;
            node_ = (bucket_idx_ < bucket_count_) ? buckets_[bucket_idx_] : nullptr;
        }
        return *this;
    }

    umap_const_iterator operator++(int) noexcept { umap_const_iterator t = *this; ++(*this); return t; }
    bool operator==(const umap_const_iterator& o) const noexcept { return node_ == o.node_; }
    bool operator!=(const umap_const_iterator& o) const noexcept { return node_ != o.node_; }
};

// ============================================================================
// unordered_map
// ============================================================================

template<typename Key,
         typename Value,
         typename Hash     = hash<Key>,
         typename KeyEqual = equal_to<Key>,
         typename Alloc    = allocator<pair<const Key, Value>>>
class unordered_map {
public:
    using key_type       = Key;
    using mapped_type    = Value;
    using value_type    = pair<const Key, Value>;
    using size_type     = size_t;
    using difference_type = decltype((char*)0 - (char*)0);
    using hasher         = Hash;
    using key_equal      = KeyEqual;
    using reference      = value_type&;
    using const_reference = const value_type&;
    using iterator       = umap_iterator<Key, Value>;
    using const_iterator = umap_const_iterator<Key, Value>;

private:
    using node_type  = hash_node<Key, Value>;
    using node_alloc = typename Alloc::template rebind<node_type>::other;
    using ptr_alloc  = typename Alloc::template rebind<node_type*>::other;

    static constexpr size_t INITIAL_BUCKETS = 16;
    static constexpr float  MAX_LOAD_FACTOR = 1.0f;

    node_type** buckets_;
    size_type   bucket_count_;
    size_type   size_;
    Hash        hasher_;
    KeyEqual    key_eq_;
    node_alloc  node_alloc_;
    ptr_alloc   ptr_alloc_;

    size_type bucket_index(size_t hv) const noexcept {
        return hv & (bucket_count_ - 1);
    }

    node_type** alloc_buckets(size_type c) {
        node_type** a = ptr_alloc_.allocate(c);
        for (size_type i = 0; i < c; ++i) a[i] = nullptr;
        return a;
    }

    void free_buckets(node_type** a, size_type c) { ptr_alloc_.deallocate(a, c); }

    template<typename... Args>
    node_type* make_node(size_t hv, Args&&... args) {
        node_type* n = node_alloc_.allocate(1);
        node_alloc_.construct(n, hv, static_cast<Args&&>(args)...);
        return n;
    }

    void free_node(node_type* n) {
        node_alloc_.destroy(n);
        node_alloc_.deallocate(n, 1);
    }

    void rehash(size_type new_bc) {
        if (new_bc <= bucket_count_) return;
        node_type** nb = alloc_buckets(new_bc);
        for (size_type i = 0; i < bucket_count_; ++i) {
            node_type* cur = buckets_[i];
            while (cur) {
                node_type* nxt = cur->next;
                size_t ni = cur->hash_val & (new_bc - 1);
                cur->next = nb[ni];
                nb[ni] = cur;
                cur = nxt;
            }
        }
        free_buckets(buckets_, bucket_count_);
        buckets_ = nb;
        bucket_count_ = new_bc;
    }

    void check_load() {
        if (static_cast<float>(size_) > static_cast<float>(bucket_count_) * MAX_LOAD_FACTOR) {
            rehash(bucket_count_ * 2);
        }
    }

    node_type* find_node(const Key& key) const noexcept {
        size_t hv = hasher_(key);
        size_t bi = bucket_index(hv);
        node_type* cur = buckets_[bi];
        while (cur) {
            if (cur->hash_val == hv && key_eq_(cur->kv.first, key)) return cur;
            cur = cur->next;
        }
        return nullptr;
    }

    size_type first_bucket() const noexcept {
        for (size_type i = 0; i < bucket_count_; ++i) if (buckets_[i]) return i;
        return bucket_count_;
    }

public:
    unordered_map()
        : buckets_(nullptr), bucket_count_(INITIAL_BUCKETS), size_(0),
          hasher_(), key_eq_(), node_alloc_(), ptr_alloc_() {
        buckets_ = alloc_buckets(bucket_count_);
    }

    unordered_map(const unordered_map& o)
        : buckets_(nullptr), bucket_count_(o.bucket_count_), size_(0),
          hasher_(o.hasher_), key_eq_(o.key_eq_),
          node_alloc_(o.node_alloc_), ptr_alloc_(o.ptr_alloc_) {
        buckets_ = alloc_buckets(bucket_count_);
        for (size_type i = 0; i < o.bucket_count_; ++i) {
            node_type* cur = o.buckets_[i];
            while (cur) { insert(cur->kv); cur = cur->next; }
        }
    }

    unordered_map(unordered_map&& o) noexcept
        : buckets_(o.buckets_), bucket_count_(o.bucket_count_), size_(o.size_),
          hasher_(zen::move(o.hasher_)), key_eq_(zen::move(o.key_eq_)),
          node_alloc_(zen::move(o.node_alloc_)), ptr_alloc_(zen::move(o.ptr_alloc_)) {
        o.buckets_ = nullptr; o.bucket_count_ = 0; o.size_ = 0;
    }

    ~unordered_map() { clear(); if (buckets_) free_buckets(buckets_, bucket_count_); }

    unordered_map& operator=(const unordered_map& o) {
        if (this != &o) {
            unordered_map t(o);
            zen::swap(buckets_, t.buckets_);
            zen::swap(bucket_count_, t.bucket_count_);
            zen::swap(size_, t.size_);
            zen::swap(hasher_, t.hasher_);
            zen::swap(key_eq_, t.key_eq_);
            zen::swap(node_alloc_, t.node_alloc_);
            zen::swap(ptr_alloc_, t.ptr_alloc_);
        }
        return *this;
    }

    unordered_map& operator=(unordered_map&& o) noexcept {
        if (this != &o) {
            clear(); if (buckets_) free_buckets(buckets_, bucket_count_);
            buckets_ = o.buckets_; bucket_count_ = o.bucket_count_; size_ = o.size_;
            hasher_ = zen::move(o.hasher_); key_eq_ = zen::move(o.key_eq_);
            node_alloc_ = zen::move(o.node_alloc_); ptr_alloc_ = zen::move(o.ptr_alloc_);
            o.buckets_ = nullptr; o.bucket_count_ = 0; o.size_ = 0;
        }
        return *this;
    }

    bool      empty()    const noexcept { return size_ == 0; }
    size_type size()     const noexcept { return size_; }
    size_type bucket_count() const noexcept { return bucket_count_; }
    float     load_factor() const noexcept {
        return bucket_count_ ? static_cast<float>(size_) / static_cast<float>(bucket_count_) : 0;
    }

    void clear() noexcept {
        for (size_type i = 0; i < bucket_count_; ++i) {
            node_type* cur = buckets_[i];
            while (cur) { node_type* nxt = cur->next; free_node(cur); cur = nxt; }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    iterator       begin() noexcept {
        size_type i = first_bucket();
        return iterator(buckets_, bucket_count_, i,
            (i < bucket_count_) ? buckets_[i] : nullptr);
    }
    const_iterator begin() const noexcept {
        size_type i = first_bucket();
        return const_iterator(buckets_, bucket_count_, i,
            (i < bucket_count_) ? buckets_[i] : nullptr);
    }
    iterator       end() noexcept { return iterator(buckets_, bucket_count_, bucket_count_, nullptr); }
    const_iterator end()   const noexcept { return const_iterator(buckets_, bucket_count_, bucket_count_, nullptr); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend()   const noexcept { return end(); }

    iterator find(const Key& k) noexcept {
        node_type* n = find_node(k);
        return iterator(buckets_, bucket_count_, n ? bucket_index(n->hash_val) : bucket_count_, n);
    }
    const_iterator find(const Key& k) const noexcept {
        node_type* n = find_node(k);
        return const_iterator(buckets_, bucket_count_, n ? bucket_index(n->hash_val) : bucket_count_, n);
    }

    bool contains(const Key& k) const noexcept { return find_node(k) != nullptr; }
    size_type count(const Key& k) const noexcept { return find_node(k) ? 1u : 0u; }

    Value& operator[](const Key& k) {
        node_type* n = find_node(k);
        if (!n) {
            auto res = insert({k, Value{}});
            return res.first->second;
        }
        return n->kv.second;
    }

    Value& operator[](Key&& k) {
        node_type* n = find_node(k);
        if (!n) {
            auto res = insert({zen::move(k), Value{}});
            return res.first->second;
        }
        return n->kv.second;
    }

    pair<iterator, bool> insert(const value_type& kv) {
        check_load();
        size_t hv = hasher_(kv.first);
        size_t bi = bucket_index(hv);
        node_type* cur = buckets_[bi];
        while (cur) {
            if (cur->hash_val == hv && key_eq_(cur->kv.first, kv.first)) {
                return {iterator(buckets_, bucket_count_, bi, cur), false};
            }
            cur = cur->next;
        }
        node_type* n = make_node(hv, kv);
        n->next = buckets_[bi];
        buckets_[bi] = n;
        ++size_;
        return {iterator(buckets_, bucket_count_, bi, n), true};
    }

    pair<iterator, bool> insert(value_type&& kv) {
        check_load();
        size_t hv = hasher_(kv.first);
        size_t bi = bucket_index(hv);
        node_type* cur = buckets_[bi];
        while (cur) {
            if (cur->hash_val == hv && key_eq_(cur->kv.first, kv.first)) {
                return {iterator(buckets_, bucket_count_, bi, cur), false};
            }
            cur = cur->next;
        }
        node_type* n = make_node(hv, zen::move(kv));
        n->next = buckets_[bi];
        buckets_[bi] = n;
        ++size_;
        return {iterator(buckets_, bucket_count_, bi, n), true};
    }

    template<typename... Args>
    pair<iterator, bool> emplace(Args&&... args) {
        return insert(value_type(static_cast<Args&&>(args)...));
    }

    size_type erase(const Key& k) {
        size_t hv = hasher_(k);
        size_t bi = bucket_index(hv);
        node_type* cur = buckets_[bi];
        node_type* prev = nullptr;
        while (cur) {
            if (cur->hash_val == hv && key_eq_(cur->kv.first, k)) {
                if (prev) prev->next = cur->next;
                else buckets_[bi] = cur->next;
                free_node(cur);
                --size_;
                return 1;
            }
            prev = cur;
            cur = cur->next;
        }
        return 0;
    }

    iterator erase(iterator pos) {
        if (!pos.node_) return pos;
        size_t bi = bucket_index(pos.node_->hash_val);
        node_type* cur = buckets_[bi];
        node_type* prev = nullptr;
        while (cur && cur != pos.node_) {
            prev = cur;
            cur = cur->next;
        }
        if (cur) {
            if (prev) prev->next = cur->next;
            else buckets_[bi] = cur->next;
            free_node(cur);
            --size_;
        }
        ++pos;
        return pos;
    }

    hasher    hash_function()    const noexcept { return hasher_; }
    key_equal key_eq_function()  const noexcept { return key_eq_; }
};

// ============================================================================
// unordered_set
// ============================================================================

template<typename Key, typename Hash = hash<Key>, typename KeyEqual = equal_to<Key>>
class unordered_set {
public:
    using key_type      = Key;
    using value_type    = Key;
    using size_type     = size_t;
    using iterator      = umap_iterator<Key, unit_t>;
    using const_iterator = umap_const_iterator<Key, unit_t>;

private:
    unordered_map<Key, unit_t, Hash, KeyEqual> impl_;

public:
    unordered_set() = default;

    bool      empty() const noexcept { return impl_.empty(); }
    size_type size()  const noexcept { return impl_.size(); }

    iterator       begin()  noexcept { return impl_.begin(); }
    const_iterator begin()  const noexcept { return impl_.begin(); }
    iterator       end()    noexcept { return impl_.end(); }
    const_iterator end()    const noexcept { return impl_.end(); }

    pair<iterator, bool> insert(const Key& k) { return impl_.insert({k, unit_t{}}); }
    pair<iterator, bool> insert(Key&& k)       { return impl_.insert({zen::move(k), unit_t{}}); }
    size_type erase(const Key& k) { return impl_.erase(k); }
    iterator  erase(iterator pos) { return impl_.erase(pos); }
    void      clear() noexcept { impl_.clear(); }

    bool      contains(const Key& k) const noexcept { return impl_.contains(k); }
    size_type count(const Key& k)    const noexcept { return impl_.count(k); }
    iterator  find(const Key& k)     noexcept { return impl_.find(k); }
};

} // namespace zen

#endif // ZEN_CONTAINERS_ASSOCIATIVE_UNORDERED_MAP_H
