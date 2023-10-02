#include <algorithm>
#include <cstddef>
#include <functional>
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

    FlatMap() :
        vec_(),
        comp_() {
    }

    T& at(const Key& key) {

    }

    bool empty() const noexcept {
        return vec_.empty();
    }
    size_type size() const noexcept {
        return vec_.size();
    }

    void clear() noexcept {
        vec_.clear();
    }
    std::pair<iterator, bool> insert(const value_type& value) {
        return emplace(value);
    }
    std::pair<iterator, bool> insert(value_type&& value) {
        return emplace(std::move(value));
    }
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type&& value(std::forward<Args>(args)...);    // FIXME is this the right idea? #########################

        std::cout << "FlatMap::emplace({" << value.first << ", " << value.second << "})\n";
        auto pos = std::lower_bound(vec_.begin(), vec_.end(), value, comp_);
        std::cout << "lower_bound found at " << pos - vec_.begin() << "\n";
        if (pos == vec_.end() || comp_(value, *pos)) {
            std::cout << "new element added\n";
            return std::make_pair(vec_.insert(pos, std::move(value)), true);
        } else {
            std::cout << "duplicate detected.\n";
            return std::make_pair(pos, false);
        }
    }
    iterator erase(iterator pos) {

    }

    iterator find(const Key& key) {
        
    }

    std::vector<value_type>& debugGetVec() {
        return vec_;
    }

private:
    struct ValueComparator {
        inline bool operator()(const value_type& lhs, const value_type& rhs) {
            Compare c;    // FIXME should this be instantiated as a member instead? ########################
            return c(lhs.first, rhs.first);
        }
    };

    std::vector<value_type> vec_;
    ValueComparator comp_;
};
