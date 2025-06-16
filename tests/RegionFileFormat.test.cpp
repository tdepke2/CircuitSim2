#include <RegionFileFormat.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <vector>

// Must specify stream converters before including catch2.
template<typename Key, typename Value, typename Compare>
std::ostream& operator<<(std::ostream& out, const std::map<Key, Value, Compare>& map) {
    out << "{ ";
    size_t i = 0;
    for (const auto& v : map) {
        out << (i++ == 0 ? "" : ", ") << "[" << v.first << "] -> " << v.second;
    }
    return out << " }";
}

#include <catch2/catch.hpp>

using SectorMap = std::map<RegionSectorPool::SectorOffset, unsigned int>;

TEST_CASE("Test sector pool ctor", "[RegionFileFormat]") {
    RegionFileFormat::ChunkHeader header = {};

    SECTION("Empty header") {
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {16, std::numeric_limits<unsigned int>::max() - 16}
        }));
    }

    SECTION("Sectors before offset 16 fail") {
        header[0].offset = 0;
        header[0].sectors = 1;
        REQUIRE_THROWS_WITH([&header]() {
            RegionSectorPool pool(header);
        }(), Catch::Contains("begins before a free sector"));

        header[0].offset = 15;
        header[0].sectors = 1;
        REQUIRE_THROWS_WITH([&header]() {
            RegionSectorPool pool(header);
        }(), Catch::Contains("begins before a free sector"));
    }

    SECTION("Sectors at and after offset 16 succeed") {
        header[0].offset = 16;
        header[0].sectors = 1;
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {17, std::numeric_limits<unsigned int>::max() - 17}
        }));

        header[0].offset = 17;
        header[0].sectors = 1;
        RegionSectorPool pool2(header);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {16, 1},
            {18, std::numeric_limits<unsigned int>::max() - 18}
        }));

        header[0].offset = 18;
        header[0].sectors = 1;
        RegionSectorPool pool3(header);
        REQUIRE(pool3.getFreeSectors() == SectorMap({
            {16, 2},
            {19, std::numeric_limits<unsigned int>::max() - 19}
        }));

        header[0].offset = 0xffffff;
        header[0].sectors = 1;
        RegionSectorPool pool4(header);
        REQUIRE(pool4.getFreeSectors() == SectorMap({
            {16, 0xffffef},
            {0x1000000, std::numeric_limits<unsigned int>::max() - 0x1000000}
        }));
    }

    SECTION("Allocate at start of free sectors") {
        header[0].offset = 32;
        header[0].sectors = 1;

        header[1].offset = 16;
        header[1].sectors = 4;
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {20, 12},
            {33, std::numeric_limits<unsigned int>::max() - 33}
        }));

        header[1].offset = 16;
        header[1].sectors = 15;
        RegionSectorPool pool2(header);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {31, 1},
            {33, std::numeric_limits<unsigned int>::max() - 33}
        }));

        header[1].offset = 16;
        header[1].sectors = 16;
        RegionSectorPool pool3(header);
        REQUIRE(pool3.getFreeSectors() == SectorMap({
            {33, std::numeric_limits<unsigned int>::max() - 33}
        }));

        header[1].offset = 16;
        header[1].sectors = 17;
        REQUIRE_THROWS_WITH([&header]() {
            RegionSectorPool pool4(header);
        }(), Catch::Contains("requires more sectors than available"));
    }

    SECTION("Allocate after start of free sectors") {
        header[0].offset = 32;
        header[0].sectors = 1;

        header[1].offset = 17;
        header[1].sectors = 4;
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {16, 1},
            {21, 11},
            {33, std::numeric_limits<unsigned int>::max() - 33}
        }));

        header[1].offset = 17;
        header[1].sectors = 14;
        RegionSectorPool pool2(header);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {16, 1},
            {31, 1},
            {33, std::numeric_limits<unsigned int>::max() - 33}
        }));

        header[1].offset = 17;
        header[1].sectors = 15;
        RegionSectorPool pool3(header);
        REQUIRE(pool3.getFreeSectors() == SectorMap({
            {16, 1},
            {33, std::numeric_limits<unsigned int>::max() - 33}
        }));

        header[1].offset = 17;
        header[1].sectors = 16;
        REQUIRE_THROWS_WITH([&header]() {
            RegionSectorPool pool4(header);
        }(), Catch::Contains("requires unavailable sectors"));
    }

    SECTION("Max allocation") {
        for (int i = 0; i < static_cast<int>(header.size()); ++i) {
            header[i].offset = i * 255 + 16;
            header[i].sectors = 255;
        }
        RegionSectorPool pool(header);
        unsigned int x = 1023 * 255 + 16 + 255;
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {x, std::numeric_limits<unsigned int>::max() - x}
        }));

        // Try in reverse order too.
        for (int i = 0; i < static_cast<int>(header.size()); ++i) {
            header[i].offset = (1023 - i) * 255 + 16;
            header[i].sectors = 255;
        }
        RegionSectorPool pool2(header);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {x, std::numeric_limits<unsigned int>::max() - x}
        }));
    }
}

TEST_CASE("Test sector pool allocate", "[RegionFileFormat]") {
    RegionFileFormat::ChunkHeader header = {};

    SECTION("Start with empty header") {
        RegionSectorPool pool(header);
        REQUIRE(pool.allocateSectors(3) == 16);
        REQUIRE(pool.allocateSectors(1) == 19);
        REQUIRE(pool.allocateSectors(10) == 20);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {30, std::numeric_limits<unsigned int>::max() - 30}
        }));
    }

    SECTION("Exceeding max allocation fails") {
        RegionSectorPool pool(header);
        REQUIRE(pool.allocateSectors(0xffffef) == 16);
        REQUIRE(pool.allocateSectors(1) == 0xffffff);
        REQUIRE_THROWS_WITH([&pool]() {
            pool.allocateSectors(1);
        }(), Catch::Contains("failed to allocate sectors"));
    }

    SECTION("Allocate with various sizes") {
        header[0].offset = 32;
        header[0].sectors = 2;
        header[1].offset = 40;
        header[1].sectors = 7;
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {16, 16},
            {34, 6},
            {47, std::numeric_limits<unsigned int>::max() - 47}
        }));
        REQUIRE(pool.allocateSectors(17) == 47);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {16, 16},
            {34, 6},
            {64, std::numeric_limits<unsigned int>::max() - 64}
        }));
        REQUIRE(pool.allocateSectors(5) == 16);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {21, 11},
            {34, 6},
            {64, std::numeric_limits<unsigned int>::max() - 64}
        }));
        REQUIRE(pool.allocateSectors(11) == 21);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {34, 6},
            {64, std::numeric_limits<unsigned int>::max() - 64}
        }));
        REQUIRE(pool.allocateSectors(6) == 34);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {64, std::numeric_limits<unsigned int>::max() - 64}
        }));
    }
}

TEST_CASE("Test sector pool free", "[RegionFileFormat]") {
    RegionFileFormat::ChunkHeader header = {};

    SECTION("Attempt to free sectors in header fails") {
        RegionSectorPool pool(header);
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(0, 1);
        }(), Catch::Contains("attempt to free sectors within header"));
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(15, 1);
        }(), Catch::Contains("attempt to free sectors within header"));
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(16, 1);
        }(), Catch::Contains("attempt to free leading sectors that are already free"));
    }

    SECTION("Merge with trailing sectors") {
        header[0].offset = 16;
        header[0].sectors = 20;
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {36, std::numeric_limits<unsigned int>::max() - 36}
        }));
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(17, 20);
        }(), Catch::Contains("attempt to free trailing sectors that are already free"));
        pool.freeSectors(17, 19);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {17, std::numeric_limits<unsigned int>::max() - 17}
        }));

        // No merge case.
        RegionSectorPool pool2(header);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {36, std::numeric_limits<unsigned int>::max() - 36}
        }));
        pool2.freeSectors(17, 18);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {17, 18},
            {36, std::numeric_limits<unsigned int>::max() - 36}
        }));
    }

    SECTION("Merge with leading sectors") {
        header[0].offset = 16;
        header[0].sectors = 10;
        header[1].offset = 32;
        header[1].sectors = 20;
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {26, 6},
            {52, std::numeric_limits<unsigned int>::max() - 52}
        }));
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(31, 19);
        }(), Catch::Contains("attempt to free leading sectors that are already free"));
        pool.freeSectors(32, 19);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {26, 25},
            {52, std::numeric_limits<unsigned int>::max() - 52}
        }));

        // No merge case.
        RegionSectorPool pool2(header);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {26, 6},
            {52, std::numeric_limits<unsigned int>::max() - 52}
        }));
        pool2.freeSectors(33, 18);
        REQUIRE(pool2.getFreeSectors() == SectorMap({
            {26, 6},
            {33, 18},
            {52, std::numeric_limits<unsigned int>::max() - 52}
        }));
    }

    SECTION("Merge on both sides") {
        header[0].offset = 16;
        header[0].sectors = 10;
        header[1].offset = 32;
        header[1].sectors = 20;
        RegionSectorPool pool(header);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {26, 6},
            {52, std::numeric_limits<unsigned int>::max() - 52}
        }));
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(31, 21);
        }(), Catch::Contains("attempt to free leading sectors that are already free"));
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(32, 21);
        }(), Catch::Contains("attempt to free trailing sectors that are already free"));
        REQUIRE_THROWS_WITH([&pool]() {
            pool.freeSectors(31, 22);
        }(), Catch::Contains("attempt to free trailing sectors that are already free"));
        pool.freeSectors(32, 20);
        REQUIRE(pool.getFreeSectors() == SectorMap({
            {26, std::numeric_limits<unsigned int>::max() - 26}
        }));
    }
}

TEST_CASE("Randomly allocate and free", "[RegionFileFormat]") {
    std::mt19937 mersenneRand(512);
    std::uniform_int_distribution<> distOffset(0, 4);
    std::uniform_int_distribution<> distSectorCount(1, 10);

    // Initialize some randomly allocated sectors for the header.
    SectorMap allocatedSectors;
    std::vector<RegionSectorPool::SectorOffset> allocatedOffsets;
    RegionSectorPool::SectorOffset lastOffset = 16;
    for (int i = 0; i < 100; ++i) {
        int offset = distOffset(mersenneRand);
        //std::cout << offset << "\n";
        lastOffset += offset;
        int count = distSectorCount(mersenneRand);
        allocatedSectors.emplace(lastOffset, count);
        allocatedOffsets.push_back(lastOffset);
        lastOffset += count;
    }

    /*std::cout << "allocatedSectors: ";
    for (const auto& x : allocatedSectors) {
        std::cout << x.first << " " << x.second << ", ";
    }
    std::cout << "\n";*/

    std::shuffle(allocatedOffsets.begin(), allocatedOffsets.end(), mersenneRand);

    /*std::cout << "after shuffling:  ";
    for (const auto& x : allocatedOffsets) {
        std::cout << x << " " << allocatedSectors.at(x) << ", ";
    }
    std::cout << "\n";*/

    RegionFileFormat::ChunkHeader header = {};
    for (size_t i = 0; i < allocatedOffsets.size(); ++i) {
        header[i].offset = allocatedOffsets[i];
        header[i].sectors = allocatedSectors.at(allocatedOffsets[i]);
    }

    auto checkFreeSectors = [](const SectorMap& sectorMap) {
        // Free sectors must begin after the header and they must have some space between each other.
        RegionSectorPool::SectorOffset lastOccupied = 15;
        for (const auto& x : sectorMap) {
            if (x.first <= lastOccupied) {
                return false;
            }
            lastOccupied = x.first + x.second;
        }
        return true;
    };

    RegionSectorPool pool(header);
    REQUIRE(checkFreeSectors(pool.getFreeSectors()));
    //std::cout << "freeSectors: " << pool.getFreeSectors() << "\n";

    // Randomly allocate and free.
    for (int i = 0; i < 100; ++i) {
        if (std::uniform_int_distribution<>(1, 10)(mersenneRand) >= 4) {
            int count = distSectorCount(mersenneRand);
            RegionSectorPool::SectorOffset offset = pool.allocateSectors(count);
            //std::cout << "allocate " << count << " sectors at " << offset << "\n";
            REQUIRE(checkFreeSectors(pool.getFreeSectors()));
            REQUIRE(allocatedSectors.emplace(offset, count).second);
            allocatedOffsets.push_back(offset);
        } else if (!allocatedOffsets.empty()) {
            size_t index = std::uniform_int_distribution<size_t>(0, allocatedOffsets.size() - 1)(mersenneRand);
            //std::cout << "free " << allocatedSectors.at(allocatedOffsets[index]) << " sectors at " << allocatedOffsets[index] << "\n";
            pool.freeSectors(allocatedOffsets[index], allocatedSectors.at(allocatedOffsets[index]));
            REQUIRE(checkFreeSectors(pool.getFreeSectors()));
            REQUIRE(allocatedSectors.erase(allocatedOffsets[index]) > 0);
            allocatedOffsets.erase(allocatedOffsets.begin() + index);
        }
    }

    //std::cout << "freeSectors after allocate and free: " << pool.getFreeSectors() << "\n";

    // Free everything in random order.
    std::shuffle(allocatedOffsets.begin(), allocatedOffsets.end(), mersenneRand);
    for (const auto& offset : allocatedOffsets) {
        //std::cout << "free " << allocatedSectors.at(offset) << " sectors at " << offset << "\n";
        pool.freeSectors(offset, allocatedSectors.at(offset));
        REQUIRE(checkFreeSectors(pool.getFreeSectors()));
    }

    REQUIRE(pool.getFreeSectors() == SectorMap({
        {16, std::numeric_limits<unsigned int>::max() - 16}
    }));
}

/**
 * add some file test cases?
 * failing case:
 * 1. add tiles in 0,0 and save
 * 2. add more tiles in 0,0 and save
 * 3. add tiles in 1,1 and save
 * 4. remove tiles in 1,1 and add tiles in 2,1 and save
 * 5. load, and we have duplicate chunks...
 * 
 * Issue is that we don't update the offset/sector count when deleting a chunk?
 */
