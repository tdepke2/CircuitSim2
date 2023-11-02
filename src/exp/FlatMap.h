#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <vector>


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

        std::stable_sort(vec_.begin(), vec_.end(), valueToValueComp_);
        auto newLast = std::unique(vec_.begin(), vec_.end(), [this](const auto& lhs, const auto& rhs) {
            return !comp_(lhs.first, rhs.first);
        });
        vec_.erase(newLast, vec_.end());

        //mergeUnique(first, last);
    }

    FlatMap(std::initializer_list<value_type> ilist, const Compare& comp = Compare()) :
        FlatMap(ilist.begin(), ilist.end(), comp) {
    }

    T& at(const Key& key) {
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
        //std::cout << "FlatMap::insert({" << value.first << ", " << value.second << "})\n";
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), value.first, valueToKeyComp_);
        //std::cout << "lower_bound found at " << pos - vec_.begin() << "\n";
        if (pos == vec_.end() || comp_(value.first, pos->first)) {
            //std::cout << "new element added\n";
            return {vec_.emplace(pos, value), true};
        } else {
            //std::cout << "duplicate detected.\n";
            return {pos, false};
        }
    }
    std::pair<iterator, bool> insert(value_type&& value) {
        //std::cout << "FlatMap::insert(rvalue ref {" << value.first << ", " << value.second << "})\n";
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), value.first, valueToKeyComp_);
        //std::cout << "lower_bound found at " << pos - vec_.begin() << "\n";
        if (pos == vec_.end() || comp_(value.first, pos->first)) {
            //std::cout << "new element added\n";
            return {vec_.emplace(pos, std::move(value)), true};
        } else {
            //std::cout << "duplicate detected.\n";
            return {pos, false};
        }
    }
    // Note: the behavior is undefined if `first` and `last` are iterators into `*this`. FIXME this should be fixed with new mergeUnique() function?
    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        if (first == last) {
            return;
        }

        auto insertStart = vec_.insert(vec_.end(), first, last);
        std::stable_sort(insertStart, vec_.end(), valueToValueComp_);
        std::inplace_merge(vec_.begin(), insertStart, vec_.end(), valueToValueComp_);
        auto newLast = std::unique(vec_.begin(), vec_.end(), [this](const auto& lhs, const auto& rhs) {
            return !comp_(lhs.first, rhs.first);
        });
        vec_.erase(newLast, vec_.end());

        //mergeUnique(first, last);
    }
    void insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        // Constructing a temporary to pass to insert() probably isn't the best
        // way to do this, boost::container::flat_map does something fancy here.
        return insert(value_type(std::forward<Args>(args)...));
    }

    // Potential improvement to emplace, requires K to match the key type.
    /*template<typename K, typename M>
    std::pair<iterator, bool> emplace(K&& key, M&& mapped) {
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), key, valueToKeyComp_);
        if (pos == vec_.end() || comp_(key, pos->first)) {
            return {vec_.emplace(pos, std::forward<K>(key), std::forward<M>(mapped)), true};
        } else {
            return {pos, false};
        }
    }*/

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
    iterator lower_bound(const Key& key) {
        return std::lower_bound(vec_.begin(), vec_.end(), key, valueToKeyComp_);
    }
    const_iterator lower_bound(const Key& key) const {
        return std::lower_bound(vec_.begin(), vec_.end(), key, valueToKeyComp_);
    }
    iterator upper_bound(const Key& key) {
        return std::upper_bound(vec_.begin(), vec_.end(), key, [this](const auto& lhs, const auto& rhs) {
            return comp_(lhs, rhs.first);
        });
    }
    const_iterator upper_bound(const Key& key) const {
        return std::upper_bound(vec_.begin(), vec_.end(), key, [this](const auto& lhs, const auto& rhs) {
            return comp_(lhs, rhs.first);
        });
    }

    friend bool operator==(const FlatMap& lhs, const FlatMap& rhs) {
        return lhs.vec_ == rhs.vec_;
    }
    friend bool operator!=(const FlatMap& lhs, const FlatMap& rhs) {
        return lhs.vec_ != rhs.vec_;
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

    //template<typename InputIt>
    //void mergeUnique(InputIt first, InputIt last);

    std::vector<value_type> vec_;
    Compare comp_;
    ValueToKeyComparator valueToKeyComp_;
    ValueToValueComparator valueToValueComp_;
};


// Tested an alternative to std::inplace_merge() and std::unique(), but resulted
// in about the same performance.
// 
// Merges a range of `value_type` into `vec_` discarding any values with
// duplicate keys. When removing duplicates, existing values in `vec_` take
// priority.
/*template<typename Key, typename T, typename Compare>
template<typename InputIt>
void FlatMap<Key, T, Compare>::mergeUnique(InputIt first, InputIt last) {
    // Ensure range 2 is sorted and has no duplicates.
    std::vector<value_type> newValues(first, last);
    std::stable_sort(newValues.begin(), newValues.end(), valueToValueComp_);
    newValues.erase(
        std::unique(newValues.begin(), newValues.end(), [this](const auto& lhs, const auto& rhs) {
            return !comp_(lhs.first, rhs.first);
        }),
        newValues.end()
    );

    std::vector<value_type> newVec;
    newVec.reserve(vec_.size() + newValues.size());

    // Merge the two ranges, discarding elements that are equal from the second range.
    iterator first1 = vec_.begin(), last1 = vec_.end();
    iterator first2 = newValues.begin(), last2 = newValues.end();
    while (first1 != last1) {
        if (first2 == last2) {
            std::move(first1, last1, std::back_inserter(newVec));
            //newVec.insert(newVec.end(), std::make_move_iterator(first1), std::make_move_iterator(last1));
            vec_.swap(newVec);
            return;
        } else if (valueToValueComp_(*first1, *first2)) {
            newVec.emplace_back(std::move(*first1));
            ++first1;
        } else if (valueToValueComp_(*first2, *first1)) {
            newVec.emplace_back(std::move(*first2));
            ++first2;
        } else {
            newVec.emplace_back(std::move(*first1));
            ++first1;
            ++first2;
        }
    }
    std::move(first2, last2, std::back_inserter(newVec));
    //newVec.insert(newVec.end(), std::make_move_iterator(first2), std::make_move_iterator(last2));
    vec_.swap(newVec);
}*/
