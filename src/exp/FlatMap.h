#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <utility>
#include <vector>



#include <iostream>


template<typename Key, typename T, typename Compare = std::less<Key>>
class FlatMap {
public:
    using key_type = Key;
    using mapped_type = T;
    using key_compare = Compare;
    using value_type = std::pair<Key, T>;
    using size_type = typename std::vector<value_type>::size_type;
    using difference_type = typename std::vector<value_type>::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;
    using reverse_iterator = typename std::vector<value_type>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<value_type>::const_reverse_iterator;

    FlatMap(const Compare& comp = Compare()) :
        vec_(),
        comp_(comp),
        valueToKeyComp_(comp),
        valueToValueComp_(comp) {
    }

    template<typename InputIt>
    FlatMap(InputIt first, InputIt last, const Compare& comp = Compare()) :
        vec_(first, last),
        comp_(comp),
        valueToKeyComp_(comp),
        valueToValueComp_(comp) {

        std::sort(vec_.begin(), vec_.end(), valueToValueComp_);
        auto newLast = std::unique(vec_.begin(), vec_.end(), [this](const auto& lhs, const auto& rhs) {
            return !comp_(lhs.first, rhs.first);
        });
        vec_.erase(newLast, vec_.end());
    }

    FlatMap(std::initializer_list<value_type> init, const Compare& comp = Compare()) :
        FlatMap(init.begin(), init.end(), comp) {
    }

    T& at(const Key& key) {    // FIXME how to avoid the code duplication with below?
        auto val = find(key);
        if (val == vec_.end()) {
            throw std::out_of_range("FlatMap::at");
        }
        return val->second;
    }
    const T& at(const Key& key) const {
        auto val = find(key);
        if (val == vec_.end()) {
            throw std::out_of_range("FlatMap::at");
        }
        return val->second;
    }
    T& operator[](const Key& key) {
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), key, valueToKeyComp_);
        if (pos == vec_.end() || comp_(key, pos->first)) {
            return vec_.emplace(pos, std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>())->second;
        } else {
            return pos->second;
        }
    }
    T& operator[](Key&& key) {
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), key, valueToKeyComp_);
        if (pos == vec_.end() || comp_(key, pos->first)) {
            return vec_.emplace(pos, std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::tuple<>())->second;
        } else {
            return pos->second;
        }
    }

    iterator begin() noexcept {
        return vec_.begin();
    }
    const_iterator begin() const noexcept {
        return vec_.begin();
    }
    const_iterator cbegin() const noexcept {
        return vec_.cbegin();
    }

    iterator end() noexcept {
        return vec_.end();
    }
    const_iterator end() const noexcept {
        return vec_.end();
    }
    const_iterator cend() const noexcept {
        return vec_.cend();
    }

    reverse_iterator rbegin() noexcept {
        return vec_.rbegin();
    }
    const_reverse_iterator rbegin() const noexcept {
        return vec_.rbegin();
    }
    const_reverse_iterator crbegin() const noexcept {
        return vec_.crbegin();
    }

    reverse_iterator rend() noexcept {
        return vec_.rend();
    }
    const_reverse_iterator rend() const noexcept {
        return vec_.rend();
    }
    const_reverse_iterator crend() const noexcept {
        return vec_.crend();
    }

    bool empty() const noexcept {
        return vec_.empty();
    }
    size_type size() const noexcept {
        return vec_.size();
    }
    size_type max_size() const noexcept {
        return vec_.max_size();
    }

    void clear() noexcept {
        vec_.clear();
    }
    std::pair<iterator, bool> insert(const value_type& value) {
        std::cout << "FlatMap::insert({" << value.first << ", " << value.second << "})\n";
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), value.first, valueToKeyComp_);
        std::cout << "lower_bound found at " << pos - vec_.begin() << "\n";
        if (pos == vec_.end() || comp_(value.first, pos->first)) {
            std::cout << "new element added\n";
            return {vec_.insert(pos, value), true};
        } else {
            std::cout << "duplicate detected.\n";
            return {pos, false};
        }
    }
    std::pair<iterator, bool> insert(value_type&& value) {
        std::cout << "FlatMap::insert(rvalue ref {" << value.first << ", " << value.second << "})\n";
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), value.first, valueToKeyComp_);
        std::cout << "lower_bound found at " << pos - vec_.begin() << "\n";
        if (pos == vec_.end() || comp_(value.first, pos->first)) {
            std::cout << "new element added\n";
            return {vec_.insert(pos, std::move(value)), true};
        } else {
            std::cout << "duplicate detected.\n";
            return {pos, false};
        }
    }
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        // Constructing a temporary to pass to insert() probably isn't the best
        // way to do this, boost::container::flat_map does something fancy here.


        // FIXME idea: use TMP to specialize two emplace functions, one accepts K, V types and other accepts value_type.
        // now we can get the key without the temporary
        // hang on, pair can be constructed with piecewise, maybe treat that as special case?


        return insert(value_type(std::forward<Args>(args)...));
    }
    iterator erase(iterator pos) {

    }

    size_type count(const Key& key) const {
        return find(key) == vec_.end() ? 0 : 1;
    }
    iterator find(const Key& key) {
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), key, valueToKeyComp_);
        if (pos == vec_.end() || comp_(key, pos->first)) {
            return vec_.end();
        } else {
            return pos;
        }
    }
    const_iterator find(const Key& key) const {
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), key, valueToKeyComp_);
        if (pos == vec_.end() || comp_(key, pos->first)) {
            return vec_.end();
        } else {
            return pos;
        }
    }

    std::vector<value_type>& debugGetVec() {
        return vec_;
    }

private:
    struct ValueToKeyComparator {
        ValueToKeyComparator(const Compare& c) :
            c(c) {
        }
        inline bool operator()(const value_type& lhs, const key_type& rhs) {
            return c(lhs.first, rhs);
        }
        Compare c;
    };
    struct ValueToValueComparator {
        ValueToValueComparator(const Compare& c) :
            c(c) {
        }
        inline bool operator()(const value_type& lhs, const value_type& rhs) {
            return c(lhs.first, rhs.first);
        }
        Compare c;
    };

    std::vector<value_type> vec_;
    Compare comp_;
    ValueToKeyComparator valueToKeyComp_;
    ValueToValueComparator valueToValueComp_;
};
