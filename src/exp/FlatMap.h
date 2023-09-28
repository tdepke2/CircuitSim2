#include <functional>
#include <utility>
#include <vector>

template<typename Key, typename T, typename Compare = std::less<Key>>
class FlatMap {
public:
    FlatMap();


private:
    std::vector<std::pair<const Key, T>> vec_;
};
