// Ensure ghc::filesystem implementation is included before header.
#include <ghc/fs_std_impl.hpp>

#include <Filesystem.h>

#include <fstream>
#include <random>
#include <string>

namespace details {

fs::path fs_mktemp(bool createDirectory, const fs::path& pattern) {
    const auto patternStr = pattern.filename().string();
    size_t xStart = 0, xEnd = 0;
    for (size_t i = patternStr.length(); i > 0;) {
        --i;
        if (patternStr[i] == 'X' && xEnd == 0) {
            xEnd = i + 1;
        } else if (patternStr[i] != 'X' && xEnd != 0) {
            xStart = i + 1;
            break;
        }
    }
    if (xEnd - xStart < 3) {
        throw fs::filesystem_error("mktemp: too few X\'s in template \'" + patternStr + "\'.", {});
    }

    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device randDevice;
    std::mt19937 randGen(randDevice());
    std::uniform_int_distribution<size_t> distribution(0, chars.length() - 1);

    fs::path parentPath;
    if (pattern.is_absolute()) {
        parentPath = pattern.parent_path();
    } else {
        parentPath = fs::temp_directory_path() / pattern.parent_path();
    }

    while (true) {
        std::string randPattern;
        for (size_t i = xStart; i < xEnd; ++i) {
            randPattern.push_back(chars[distribution(randGen)]);
        }
        fs::path p = parentPath / (patternStr.substr(0, xStart) + randPattern + patternStr.substr(xEnd));
        if (fs::exists(p)) {
            continue;
        }

        // May not be the safest way to create the file/directory since theoretically it could be created after checking existence.
        if (createDirectory) {
            if (!fs::create_directory(p)) {
                throw fs::filesystem_error("mktemp: unable to create directory \'" + p.string() + "\'.", {});
            }
        } else {
            std::ofstream outFile(p);
            if (!outFile.is_open()) {
                throw fs::filesystem_error("mktemp: unable to create file \'" + p.string() + "\'.", {});
            }
        }
        return p;
    }
}

}
