#include <LegacyFileFormat.h>

#include <fstream>

void LegacyFileFormat::loadFromFile(Board& board, const std::string& filename) {

}

void LegacyFileFormat::saveToFile(Board& board) {
    std::ofstream out("binary_test.bin", std::ios::binary);
    if (!out.is_open()) {
        // FIXME not good.
        return;
    }
    const char s[] = "\nhello";

    uint8_t x8 = 0x01;
    out.write(reinterpret_cast<char*>(&x8), sizeof(x8));
    x8 = byteswap(x8);
    out.write(reinterpret_cast<char*>(&x8), sizeof(x8));
    out.write(s, 1);

    uint16_t x16 = 0x0203;
    out.write(reinterpret_cast<char*>(&x16), sizeof(x16));
    x16 = byteswap(x16);
    out.write(reinterpret_cast<char*>(&x16), sizeof(x16));
    out.write(s, 1);

    uint32_t x32 = 0x04050607;
    out.write(reinterpret_cast<char*>(&x32), sizeof(x32));
    x32 = byteswap(x32);
    out.write(reinterpret_cast<char*>(&x32), sizeof(x32));
    out.write(s, 1);

    int xInt = 0x01020304;
    out.write(reinterpret_cast<char*>(&xInt), sizeof(xInt));
    xInt = byteswap(xInt);
    out.write(reinterpret_cast<char*>(&xInt), sizeof(xInt));
    out.write(s, 1);

    uint64_t x64 = 0x0102030405060708;
    out.write(reinterpret_cast<char*>(&x64), sizeof(x64));
    x64 = byteswap(x64);
    out.write(reinterpret_cast<char*>(&x64), sizeof(x64));
    out.write(s, 1);

    float xFloat = 123.456f;
    out.write(reinterpret_cast<char*>(&xFloat), sizeof(xFloat));
    xFloat = byteswap(xFloat);
    out.write(reinterpret_cast<char*>(&xFloat), sizeof(xFloat));
    out.write(s, 1);

    // This should not compile.
    /*int* xPtr = &xInt;
    out.write(reinterpret_cast<char*>(&xPtr), sizeof(xPtr));
    xPtr = byteswap(xPtr);
    out.write(reinterpret_cast<char*>(&xPtr), sizeof(xPtr));*/
    out.write(s, 6);
}
