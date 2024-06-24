#include <memory>

/**
 * Implementation of `std::make_unique()` for C++11. This function is useful for
 * avoiding exception safety problems when using `new` within a call to a unique
 * pointer constructor.
 * 
 * Based on MSVC implementation and the following:
 * https://stackoverflow.com/a/17902439
 */

// Alternatively, a simple minimal implementation looks like the below snippet.
// From: https://herbsutter.com/gotw/_102/
/*template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}*/

#if __cplusplus >= 201402L    // C++14 and beyond.

namespace details {
using std::make_unique;
}

#else    // C++11 version.

#include <type_traits>
#include <utility>

namespace details {

template<class T, class... Args, typename std::enable_if<!std::is_array<T>::value, int>::type = 0>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T, typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0, int>::type = 0>
std::unique_ptr<T> make_unique(size_t size) {
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[size]());
}

template<class T, class... Args, typename std::enable_if<std::extent<T>::value != 0, int>::type = 0>
void make_unique(Args&&...) = delete;

}

#endif
