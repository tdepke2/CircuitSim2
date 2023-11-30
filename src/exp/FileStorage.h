#pragma once

#include <ChunkCoords.h>
#include <Filesystem.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <type_traits>
#ifdef _MSC_VER
    #include <intrin.h>
#endif

class Board;
class ChunkCoordsRange;

class FileStorage {
public:
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
    static inline T byteswap(T n) noexcept;

    static float getFileVersion(const fs::path& filename, fs::ifstream& boardFile);

    virtual bool validateFileVersion(float version) = 0;
    virtual void loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) = 0;
    virtual void saveToFile(Board& board) = 0;

    virtual void updateVisibleChunks(Board& board, const ChunkCoordsRange& visibleChunks);
    virtual bool loadChunk(Board& board, ChunkCoords::repr chunkCoords);
};

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
