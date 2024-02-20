#include <Tile.h>
#include <TilePool.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>

#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Quick check", "[TilePool]") {
    TilePool pool;
    //pool.accessTile(0);
    auto result = pool.allocateTile();
    std::cout << "Allocate 1: tile " << static_cast<int>(result.first.getId()) << ", id " << result.second << "\n";
    result.first.setType(tiles::Wire::instance());
    std::cout << "Tile 1 (after change): " << static_cast<int>(pool.accessTile(result.second).getId()) << "\n";

    auto result2 = pool.allocateTile();
    std::cout << "Allocate 2: tile " << static_cast<int>(result2.first.getId()) << ", id " << result2.second << "\n";
    result2.first.setType(tiles::Input::instance());
    std::cout << "Tile 2 (after change): " << static_cast<int>(pool.accessTile(result2.second).getId()) << "\n";

    pool.freeTile(result.second);
    std::cout << "Free 1: done\n";
    std::cout << "Tile 2: " << static_cast<int>(pool.accessTile(result2.second).getId()) << "\n";

    pool.freeTile(result2.second);
    std::cout << "Free 2: done\n";

    auto result3 = pool.allocateTile();
    std::cout << "Allocate 3: tile " << static_cast<int>(result3.first.getId()) << ", id " << result3.second << "\n";
    result3.first.setType(tiles::Input::instance(), TileId::inButton);
    std::cout << "Tile 3 (after change): " << static_cast<int>(pool.accessTile(result3.second).getId()) << "\n";

    std::cout << "Allocating to fill chunk area...\n";
    for (int i = 3; i < 1024; ++i) {
        pool.allocateTile();
    }

    auto result1024 = pool.allocateTile();
    std::cout << "Allocate 1024: tile " << static_cast<int>(result1024.first.getId()) << ", id " << result1024.second << "\n";
    result1024.first.setType(tiles::Wire::instance(), TileId::wireCrossover);
    std::cout << "Tile 1024 (after change): " << static_cast<int>(pool.accessTile(result1024.second).getId()) << "\n";

    pool.debugPrintInfo();

    std::cout << "Freeing to clear chunk area...\n";
    for (int i = 2; i < 1024; ++i) {
        pool.freeTile(i);
    }

    auto result1025 = pool.allocateTile();
    std::cout << "Allocate 1025: tile " << static_cast<int>(result1025.first.getId()) << ", id " << result1025.second << "\n";
    result1025.first.setType(tiles::Wire::instance(), TileId::wireJunction);
    std::cout << "Tile 1025 (after change): " << static_cast<int>(pool.accessTile(result1025.second).getId()) << "\n";

    pool.debugPrintInfo();


    // FIXME: need to test null items in the middle, and removing multiple nulls at once.
    // what if we are near boundary with little to no tiles allocated, and we allocate/free?
}
