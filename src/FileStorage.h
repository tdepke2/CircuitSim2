#pragma once

#include <ChunkCoords.h>
#include <Config.h>
#include <Filesystem.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#ifdef _MSC_VER
    #include <intrin.h>
#endif

class Board;
class ChunkCoordsRange;

/**
 * Error involving file I/O. Similar to `fs::filesystem_error` but without the
 * error code.
 * 
 * In order to ensure that copy ctor and copy assignment can be noexcept, we
 * inherit from `std::runtime_error` to make use of its cow string member.
 */
class FileStorageError : public std::runtime_error {
public:
    FileStorageError(const std::string& what) :
        std::runtime_error(what) {
    }
    FileStorageError(const std::string& what, const fs::path& path) :
        std::runtime_error("\"" + path.string() + "\": " + what) {
    }
    virtual ~FileStorageError() = default;
    FileStorageError(const FileStorageError& rhs) noexcept = default;
    FileStorageError& operator=(const FileStorageError& rhs) noexcept = default;
};

/**
 * Abstract class for circuit file formats.
 * 
 * The `boardFile` parameters refer to the top-level text file that defines the
 * board properties (it begins with the "version" field).
 */
class FileStorage {
public:
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
    static inline T swapHostBigEndian(T n) noexcept;

    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
    static inline T swapHostLittleEndian(T n) noexcept;

    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
    static inline T byteswap(T n) noexcept;

    static float getFileVersion(const fs::path& filename, fs::ifstream& boardFile);

    FileStorage(const fs::path& filename);
    virtual ~FileStorage() = default;

    const fs::path& getFilename() const;
    bool isNewFile() const;

    virtual fs::path getDefaultFileExtension() const = 0;
    virtual bool validateFileVersion(float version) = 0;
    virtual void loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) = 0;
    virtual void saveToFile(Board& board) = 0;
    virtual void saveAsFile(Board& board, const fs::path& filename) = 0;

    virtual void updateVisibleChunks(Board& board, const ChunkCoordsRange& visibleChunks);
    virtual bool loadChunk(Board& board, ChunkCoords::repr chunkCoords);

protected:
    void setFilename(const fs::path& filename);
    void setNewFile(bool newFile);

private:
    fs::path filename_;
    bool newFile_;
};

// Convert between host endian and big endian.
template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type>
inline T FileStorage::swapHostBigEndian(T n) noexcept {
#if IS_BIG_ENDIAN
    return n;
#else
    return byteswap(n);
#endif
}

// Convert between host endian and little endian.
template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type>
inline T FileStorage::swapHostLittleEndian(T n) noexcept {
#if IS_BIG_ENDIAN
    return byteswap(n);
#else
    return n;
#endif
}

// Reverses bytes in the given integral type to convert endianness.
// Based on code found here (and improved to use intrinsics):
// https://mklimenko.github.io/english/2018/08/22/robust-endian-swap/
template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type>
inline T FileStorage::byteswap(T n) noexcept {
    union {
        T val;
        std::array<uint8_t, sizeof(T)> raw;
    } u;
    u.val = n;
    std::reverse(u.raw.begin(), u.raw.end());
    return u.val;
}

#ifdef _MSC_VER
static_assert(sizeof(unsigned short) == sizeof(uint16_t), "Size of unsigned short is expected to be 2 bytes.");
static_assert(sizeof(unsigned long) == sizeof(uint32_t), "Size of unsigned long is expected to be 4 bytes.");

template<>
inline uint16_t FileStorage::byteswap(uint16_t n) noexcept {
    return _byteswap_ushort(n);
}
template<>
inline int16_t FileStorage::byteswap(int16_t n) noexcept {
    return _byteswap_ushort(n);
}

template<>
inline uint32_t FileStorage::byteswap(uint32_t n) noexcept {
    return _byteswap_ulong(n);
}
template<>
inline int32_t FileStorage::byteswap(int32_t n) noexcept {
    return _byteswap_ulong(n);
}

template<>
inline uint64_t FileStorage::byteswap(uint64_t n) noexcept {
    return _byteswap_uint64(n);
}
template<>
inline int64_t FileStorage::byteswap(int64_t n) noexcept {
    return _byteswap_uint64(n);
}

#elif defined(__GNUC__) || defined(__clang__)
template<>
inline uint32_t FileStorage::byteswap(uint32_t n) noexcept {
    return __builtin_bswap32(n);
}
template<>
inline int32_t FileStorage::byteswap(int32_t n) noexcept {
    return __builtin_bswap32(n);
}

template<>
inline uint64_t FileStorage::byteswap(uint64_t n) noexcept {
    return __builtin_bswap64(n);
}
template<>
inline int64_t FileStorage::byteswap(int64_t n) noexcept {
    return __builtin_bswap64(n);
}
#endif
