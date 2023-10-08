#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <queue>
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

    // Performs an in-place merge of two consecutive sorted ranges from [first,
    // middle) to [middle, last). Elements that are not unique from the second
    // range are discarded, reducing the vector size. This is very similar to
    // running std::inplace_merge() and std::unique() on the ranges.
    void inplaceMergeUnique(iterator first, iterator middle, iterator last) {
        std::queue<value_type> buffer;
        while (middle != last) {
            if (first == middle) {

            }
            if (buffer.empty()) {
                if (valueToValueComp_(*first, *middle)) {
                    // first right where it should be
                    ++first;
                } else if (valueToValueComp_(*middle, *first)) {
                    // middle goes first
                    buffer.push(std::move(*first));
                    *first = std::move(*middle);
                    ++first;
                    ++middle;
                } else {
                    // remove middle
                    ++middle;
                }
            } else {
                if (valueToValueComp_(buffer.front(), *middle)) {
                    // buffer comes first
                    buffer.push(std::move(*first));
                    *first = std::move(buffer.front());
                    buffer.pop();
                    ++first;
                } else if (valueToValueComp_(*middle, buffer.front())) {
                    // middle goes first
                    buffer.push(std::move(*first));
                    *first = std::move(*middle);
                    ++first;
                    ++middle;
                } else {
                    // remove middle
                    ++middle;
                }
            }
        }
        while (!buffer.empty()) {
            // insert buffer at first
            *first = std::move(buffer.front());
            buffer.pop();
            ++first;
        }
        vec_.erase(first, last);
    }

/*
ex first next to middle:
 v  v
[0, 1, 2, 3, 4]
less
    v  v
[0, 1, 2, 3, 4]
...
          v  v
[0, 1, 2, 3, 4]


 v  v
[1, 0, 2, 3]
greater
    v  v
[0, -, 2, 3]    q[1]
less
       v  v
[0, 1, 2, 3]


 v  v
[2, 0, 2, 3]
greater
    v  v
[0, -, 2, 3]    q[2]
equal
    v     v
[0, 2, -, 3]


 v  v
[0, 0, 1, 2]
equal
 v     v
[0, -, 1, 2]
less
    v     v
[0, 1, -, 2]


 v  v
[0, 1, 1, 2, 2]


ex first all less:
 v        v
[0, 1, 2, 5, 6, 7]

ex middle all less:
 v        v
[4, 5, 6, 0, 1, 2]

ex mixed:
 v           v
[0, 3, 4, 5, 1, 2, 4, 6]


in conclusion, in-place merge is very complex and will probably be faster to just merge with a copy.
still worth testing to confirm this will actually be an improvement over the std::merge/std::unique method.

*/

    FlatMap(const Compare& comp = Compare()) :
        vec_(),
        comp_(comp),
        valueToKeyComp_(comp),
        valueToValueComp_(comp) {
        //vec_.reserve(10);
    }

    template<typename InputIt>
    FlatMap(InputIt first, InputIt last, const Compare& comp = Compare()) :
        vec_(first, last),
        comp_(comp),
        valueToKeyComp_(comp),
        valueToValueComp_(comp) {

        //vec_.reserve(10);
        std::stable_sort(vec_.begin(), vec_.end(), valueToValueComp_);
        auto newLast = std::unique(vec_.begin(), vec_.end(), [this](const auto& lhs, const auto& rhs) {
            return !comp_(lhs.first, rhs.first);
        });
        vec_.erase(newLast, vec_.end());
    }

    FlatMap(std::initializer_list<value_type> ilist, const Compare& comp = Compare()) :
        FlatMap(ilist.begin(), ilist.end(), comp) {
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
    // Note: the behavior is undefined if `first` and `last` are iterators into `*this`.
    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        if (first == last) {
            return;
        }
        auto insertStart = vec_.insert(vec_.end(), first, last);

        /*std::cout << "insert() presorted = {";
        for (const auto& v : vec_) {
            std::cout << "[" << v.first << "] -> " << v.second << ", ";
        }
        std::cout << "}\n";*/

        std::stable_sort(insertStart, vec_.end(), valueToValueComp_);

        /*std::cout << "insert() after sort = {";
        for (const auto& v : vec_) {
            std::cout << "[" << v.first << "] -> " << v.second << ", ";
        }
        std::cout << "}\n";*/

        std::inplace_merge(vec_.begin(), insertStart, vec_.end(), valueToValueComp_);

        /*std::cout << "insert() after merge = {";
        for (const auto& v : vec_) {
            std::cout << "[" << v.first << "] -> " << v.second << ", ";
        }
        std::cout << "}\n";*/

        auto newLast = std::unique(vec_.begin(), vec_.end(), [this](const auto& lhs, const auto& rhs) {
            return !comp_(lhs.first, rhs.first);
        });
        vec_.erase(newLast, vec_.end());
    }
    void insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
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
        return vec_.erase(pos);
    }
    iterator erase(const_iterator pos) {
        return vec_.erase(pos);
    }
    iterator erase(iterator first, iterator last) {
        return vec_.erase(first, last);
    }
    iterator erase(const_iterator first, const_iterator last) {
        return vec_.erase(first, last);
    }
    size_type erase(const Key& key) {
        auto val = find(key);
        if (val != vec_.end()) {
            vec_.erase(val);
            return 1;
        } else {
            return 0;
        }
    }
    void swap(FlatMap& other) {
        std::swap(vec_, other.vec_);
        std::swap(comp_, other.comp_);
        std::swap(valueToKeyComp_, other.valueToKeyComp_);
        std::swap(valueToValueComp_, other.valueToValueComp_);
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

    friend bool operator==(const FlatMap& lhs, const FlatMap& rhs) {
        return lhs.vec_ == rhs.vec_;
    }
    friend bool operator!=(const FlatMap& lhs, const FlatMap& rhs) {
        return lhs.vec_ != rhs.vec_;
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
