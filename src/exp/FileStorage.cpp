#include <FileStorage.h>

inline std::string pathSeparator() {
    #if _WIN32
        return "\\";
    #else
        return "/";
    #endif
}
