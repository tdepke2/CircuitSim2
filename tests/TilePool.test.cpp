#include <Tile.h>
#include <TilePool.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>

#include <catch2/catch.hpp>
#include <iomanip>
#include <iostream>
#include <spdlog/spdlog.h>
#include <vector>

std::ostream& operator<<(std::ostream& out, TileData t) {
    out << "0x" << std::setw(8) << std::setfill('0') << std::hex << *reinterpret_cast<const uint32_t*>(&t) << std::setfill(' ') << std::dec;
    return out;
}

template<typename T>
TileData setTileInPool(TilePool& pool, size_t sectorId, unsigned int offset, TileData expected, T* tileType, TileId::t tileId) {
    REQUIRE(pool.accessTile(sectorId, offset).getRawData() == expected);
    pool.accessTile(sectorId, offset).setType(tileType, tileId);
    TileData result = pool.accessTile(sectorId, offset).getRawData();
    REQUIRE(result.id == tileId);
    return result;
}

TEST_CASE("Test allocate/free", "[TilePool]") {
    TilePool pool;
    REQUIRE(pool.getTotalAllocated() == 0);
    TileData blankTile = {};

    SECTION("Allocate/free blanks") {
        auto s0 = pool.allocateSector();
        REQUIRE(pool.getTotalAllocated() == 1);
        for (int offset = 0; offset < TilePool::SECTOR_SIZE; ++offset) {
            REQUIRE(pool.accessTile(s0, offset).getRawData() == blankTile);
        }

        auto s1 = pool.allocateSector();
        REQUIRE(pool.getTotalAllocated() == 2);
        for (int offset = 0; offset < TilePool::SECTOR_SIZE; ++offset) {
            REQUIRE(pool.accessTile(s1, offset).getRawData() == blankTile);
        }

        pool.freeSector(s0);
        REQUIRE(pool.getTotalAllocated() == 1);
        pool.freeSector(s1);
        REQUIRE(pool.getTotalAllocated() == 0);
    }

    SECTION("Allocate/free and set tiles") {
        constexpr int x = TilePool::SECTOR_SIZE - 1;

        auto s0 = pool.allocateSector();
        REQUIRE(pool.getTotalAllocated() == 1);
        auto td0 = setTileInPool(pool, s0, 0, blankTile, tiles::Wire::instance(), TileId::wireStraight);
        auto td1 = setTileInPool(pool, s0, x / 2, blankTile, tiles::Wire::instance(), TileId::wireCorner);
        auto td2 = setTileInPool(pool, s0, x, blankTile, tiles::Wire::instance(), TileId::wireTee);

        auto s1 = pool.allocateSector();
        REQUIRE(pool.getTotalAllocated() == 2);
        auto td3 = setTileInPool(pool, s1, 0, blankTile, tiles::Wire::instance(), TileId::wireJunction);
        auto td4 = setTileInPool(pool, s1, 1, blankTile, tiles::Wire::instance(), TileId::wireCrossover);
        auto td5 = setTileInPool(pool, s1, x, blankTile, tiles::Input::instance(), TileId::inSwitch);

        REQUIRE(pool.accessTile(s0, 0).getRawData() == td0);
        REQUIRE(pool.accessTile(s0, x / 2).getRawData() == td1);
        REQUIRE(pool.accessTile(s0, x).getRawData() == td2);

        pool.freeSector(s0);
        REQUIRE(pool.getTotalAllocated() == 1);

        auto s2 = pool.allocateSector();
        REQUIRE(pool.getTotalAllocated() == 2);
        auto td6 = setTileInPool(pool, s2, 0, td0, tiles::Input::instance(), TileId::inButton);
        auto td7 = setTileInPool(pool, s2, x / 2, td1, tiles::Gate::instance(), TileId::gateDiode);
        auto td8 = setTileInPool(pool, s2, x, td2, tiles::Gate::instance(), TileId::gateBuffer);

        auto s3 = pool.allocateSector();
        REQUIRE(pool.getTotalAllocated() == 3);
        auto td9 = setTileInPool(pool, s3, 0, blankTile, tiles::Gate::instance(), TileId::gateNot);
        auto td10 = setTileInPool(pool, s3, x - 1, blankTile, tiles::Gate::instance(), TileId::gateAnd);
        auto td11 = setTileInPool(pool, s3, x, blankTile, tiles::Gate::instance(), TileId::gateNand);

        REQUIRE(pool.accessTile(s0, 0).getRawData() == td6);
        REQUIRE(pool.accessTile(s0, x / 2).getRawData() == td7);
        REQUIRE(pool.accessTile(s0, x).getRawData() == td8);
        REQUIRE(pool.accessTile(s1, 0).getRawData() == td3);
        REQUIRE(pool.accessTile(s1, 1).getRawData() == td4);
        REQUIRE(pool.accessTile(s1, x).getRawData() == td5);
        REQUIRE(pool.accessTile(s3, 0).getRawData() == td9);
        REQUIRE(pool.accessTile(s3, x - 1).getRawData() == td10);
        REQUIRE(pool.accessTile(s3, x).getRawData() == td11);
    }

    SECTION("Allocate/free a lot") {
        constexpr int x = TilePool::SECTOR_SIZE - 1;

        std::vector<size_t> sectors;
        for (unsigned int i = 0; i < 500; ++i) {
            sectors.push_back(pool.allocateSector());
            REQUIRE(pool.getTotalAllocated() == i + 1);
        }

        auto td0 = setTileInPool(pool, sectors[0], 0, blankTile, tiles::Wire::instance(), TileId::wireStraight);
        auto td1 = setTileInPool(pool, sectors[0], x / 2, blankTile, tiles::Wire::instance(), TileId::wireCorner);
        auto td2 = setTileInPool(pool, sectors[0], x, blankTile, tiles::Wire::instance(), TileId::wireTee);

        auto td3 = setTileInPool(pool, sectors[1], 0, blankTile, tiles::Wire::instance(), TileId::wireJunction);
        auto td4 = setTileInPool(pool, sectors[1], 3, blankTile, tiles::Wire::instance(), TileId::wireCrossover);
        auto td5 = setTileInPool(pool, sectors[1], x, blankTile, tiles::Input::instance(), TileId::inSwitch);

        auto td6 = setTileInPool(pool, sectors[497], 0, blankTile, tiles::Input::instance(), TileId::inButton);
        auto td7 = setTileInPool(pool, sectors[497], x - 3, blankTile, tiles::Gate::instance(), TileId::gateDiode);
        auto td8 = setTileInPool(pool, sectors[497], x, blankTile, tiles::Gate::instance(), TileId::gateBuffer);

        pool.freeSector(sectors[497]);
        REQUIRE(pool.getTotalAllocated() == 499);
        pool.freeSector(sectors[499]);
        REQUIRE(pool.getTotalAllocated() == 498);
        pool.freeSector(sectors[498]);
        REQUIRE(pool.getTotalAllocated() == 497);
        sectors.resize(497);

        REQUIRE(pool.accessTile(sectors.back(), 0).getRawData() == blankTile);
        sectors.push_back(pool.allocateSector());
        REQUIRE(pool.getTotalAllocated() == 498);
        REQUIRE(pool.accessTile(sectors[497], 0).getRawData() == td6);
        REQUIRE(pool.accessTile(sectors[497], x - 3).getRawData() == td7);
        REQUIRE(pool.accessTile(sectors[497], x).getRawData() == td8);

        for (int i = 1; i < 200; ++i) {
            pool.freeSector(sectors[i]);
            REQUIRE(pool.getTotalAllocated() == static_cast<unsigned int>(498 - i));
        }
        for (int i = 497; i >= 200; --i) {
            pool.freeSector(sectors[i]);
            REQUIRE(pool.getTotalAllocated() == static_cast<unsigned int>(i - 199));
        }
        sectors.resize(1);

        sectors.push_back(pool.allocateSector());
        REQUIRE(pool.getTotalAllocated() == 2);
        auto td9 = setTileInPool(pool, sectors[1], 0, td3, tiles::Gate::instance(), TileId::gateNot);
        auto td10 = setTileInPool(pool, sectors[1], 4, blankTile, tiles::Gate::instance(), TileId::gateAnd);
        auto td11 = setTileInPool(pool, sectors[1], x, td5, tiles::Gate::instance(), TileId::gateNand);

        REQUIRE(pool.accessTile(sectors[0], 0).getRawData() == td0);
        REQUIRE(pool.accessTile(sectors[0], x / 2).getRawData() == td1);
        REQUIRE(pool.accessTile(sectors[0], x).getRawData() == td2);
        REQUIRE(pool.accessTile(sectors[1], 0).getRawData() == td9);
        REQUIRE(pool.accessTile(sectors[1], 3).getRawData() == td4);
        REQUIRE(pool.accessTile(sectors[1], 4).getRawData() == td10);
        REQUIRE(pool.accessTile(sectors[1], x).getRawData() == td11);
    }
}

TEST_CASE("Manual test", "[.][TilePool]") {
    spdlog::set_level(spdlog::level::debug);

    TilePool pool;
    pool.debugPrintInfo();

    //pool.accessTile(0, 0);
    size_t s0 = pool.allocateSector();
    //pool.accessTile(s0, TilePool::SECTOR_SIZE);
    Tile t00 = pool.accessTile(s0, 0);
    spdlog::debug("Allocate 1: sector {}, tile 0 is {}.", s0, static_cast<int>(t00.getId()));
    t00.setType(tiles::Wire::instance());
    spdlog::debug("After change 1: tile 0 is {}.", static_cast<int>(t00.getId()));
    pool.accessTile(s0, (TilePool::SECTOR_SIZE - 1) / 2).setType(tiles::Wire::instance(), TileId::wireTee);
    pool.accessTile(s0, TilePool::SECTOR_SIZE - 1).setType(tiles::Wire::instance(), TileId::wireCrossover);

    pool.debugPrintInfo();

    size_t s1 = pool.allocateSector();
    Tile t10 = pool.accessTile(s1, 0);
    spdlog::debug("Allocate 2: sector {}, tile 0 is {}.", s1, static_cast<int>(t10.getId()));
    t10.setType(tiles::Input::instance());
    spdlog::debug("After change 2: tile 0 is {}.", static_cast<int>(t10.getId()));

    for (int i = 0; i < 63; ++i) {
        pool.allocateSector();
        pool.debugPrintInfo();
    }

    size_t s65 = pool.allocateSector();
    Tile t650 = pool.accessTile(s65, 0);
    spdlog::debug("Allocate 65: sector {}, tile 0 is {}.", s65, static_cast<int>(t650.getId()));
    t650.setType(tiles::Led::instance());
    spdlog::debug("After change 65: tile 0 is {}.", static_cast<int>(t650.getId()));

    pool.freeSector(0);
    size_t s66 = pool.allocateSector();
    Tile t660 = pool.accessTile(s66, 0);
    spdlog::debug("Allocate 66: sector {}, tile 0 is {}.", s66, static_cast<int>(t660.getId()));
    t660.setType(tiles::Gate::instance());
    spdlog::debug("After change 66: tile 0 is {}.", static_cast<int>(t660.getId()));

    pool.debugPrintInfo();

    t00.getChunk().debugPrintChunk();
    t650.getChunk().debugPrintChunk();
}
