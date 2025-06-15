set(CS2_COMMANDS_SRCS
    commands/EditTiles.cpp
    commands/EditTiles.h
    commands/FillArea.cpp
    commands/FillArea.h
    commands/FlipTiles.cpp
    commands/FlipTiles.h
    commands/PlaceTiles.cpp
    commands/PlaceTiles.h
    commands/RotateTiles.cpp
    commands/RotateTiles.h
    commands/ToggleTiles.cpp
    commands/ToggleTiles.h
)

set(CS2_ENTITIES_SRCS
    entities/Label.cpp
    entities/Label.h
)

set(CS2_TILES_SRCS
    tiles/Blank.cpp
    tiles/Blank.h
    tiles/Gate.cpp
    tiles/Gate.h
    tiles/Input.cpp
    tiles/Input.h
    tiles/Label.cpp
    tiles/Label.h
    tiles/Led.cpp
    tiles/Led.h
    tiles/Wire.cpp
    tiles/Wire.h
)

# Create the main library from all source files.
add_library(cs2_src
    ${CS2_COMMANDS_SRCS}
    ${CS2_ENTITIES_SRCS}
    ${CS2_TILES_SRCS}
    Board.cpp
    Board.h
    Chunk.cpp
    Chunk.h
    ChunkCoords.h
    ChunkCoordsRange.h
    ChunkDrawable.cpp
    ChunkDrawable.h
    ChunkRender.cpp
    ChunkRender.h
    Command.cpp
    Command.h
    DebugScreen.cpp
    DebugScreen.h
    Editor.cpp
    Editor.h
    EditorInterface.cpp
    EditorInterface.h
    Entity.cpp
    Entity.h
    FileStorage.cpp
    FileStorage.h
    Filesystem.cpp
    Filesystem.h
    FlatMap.h
    LegacyFileFormat.cpp
    LegacyFileFormat.h
    Locator.cpp
    Locator.h
    LodRenderer.cpp
    LodRenderer.h
    MakeUnique.h
    MessageLogSink.h
    OffsetView.cpp
    OffsetView.h
    RegionFileFormat.cpp
    RegionFileFormat.h
    ResourceBase.h
    ResourceManager.cpp
    ResourceManager.h
    SubBoard.cpp
    SubBoard.h
    Tile.cpp
    Tile.h
    TilePool.cpp
    TilePool.h
    TileType.cpp
    TileType.h
    TileWidth.h
)

# Organize source and header files for IDEs.
source_group("Header Files/commands" REGULAR_EXPRESSION "commands/.*\.h")
source_group("Source Files/commands" REGULAR_EXPRESSION "commands/.*\.cpp")
source_group("Header Files/entities" REGULAR_EXPRESSION "entities/.*\.h")
source_group("Source Files/entities" REGULAR_EXPRESSION "entities/.*\.cpp")
source_group("Header Files/tiles" REGULAR_EXPRESSION "tiles/.*\.h")
source_group("Source Files/tiles" REGULAR_EXPRESSION "tiles/.*\.cpp")

# Specify where to find the header files to include.
target_include_directories(cs2_src PUBLIC .)

# Link to libraries and set C++ compilation version.
target_link_libraries(cs2_src PUBLIC
    cs2_gui
    sfml-graphics
    portable_file_dialogs
    spdlog::spdlog
    ghc_filesystem
)
cs2_add_cxx_properties(cs2_src)

# Write version to config header and determine host endianness.
if(${CMAKE_VERSION} VERSION_LESS 3.20)
    include(TestBigEndian)
    TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
elseif(CMAKE_CXX_BYTE_ORDER STREQUAL "BIG_ENDIAN")
    set(IS_BIG_ENDIAN 1)
else()
    set(IS_BIG_ENDIAN 0)
endif()
if(IS_BIG_ENDIAN)
    message(STATUS "Configuring for big endian.")
else()
    message(STATUS "Configuring for little endian.")
endif()
configure_file(
    "${PROJECT_SOURCE_DIR}/src/Config.h.in"
    "${PROJECT_BINARY_DIR}/src/Config.h"
)
target_include_directories(cs2_src PUBLIC "${PROJECT_BINARY_DIR}/src")

add_executable(CircuitSim2_experimental
    main/main.cpp
    $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>:main/MainWin32.cpp>
)
target_link_libraries(CircuitSim2_experimental PRIVATE cs2_src)
cs2_add_cxx_properties(CircuitSim2_experimental)

# On Windows, disable console from opening with application (except when the
# build type is Debug).
# See: https://www.sfml-dev.org/faq.php#tr-win-console
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(CircuitSim2_experimental PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)

    # We could link against sfml-main to have the WinMain entry point provided.
    # Instead, a customized WinMain is used that does console redirection.
    #target_link_libraries(CircuitSim2_experimental PRIVATE sfml-main)
endif()

# Build the gui.
add_subdirectory(gui)

# Build the old (stable) app.
add_subdirectory(deadcode)
