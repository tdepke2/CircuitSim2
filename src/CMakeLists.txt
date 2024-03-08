set(CS2_COMMANDS_SRCS
    commands/FillArea.cpp
    commands/FillArea.h
    commands/PlaceTiles.cpp
    commands/PlaceTiles.h
    commands/RotateTiles.cpp
    commands/RotateTiles.h
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
    sfml-graphics
    spdlog::spdlog
    ghc_filesystem
)
cs2_add_cxx_properties(cs2_src)

add_executable(CircuitSim2_experimental
    main.cpp
)
target_link_libraries(CircuitSim2_experimental PRIVATE cs2_src)
cs2_add_cxx_properties(CircuitSim2_experimental)

# Build the gui.
add_subdirectory(gui)

# Build the old (stable) app.
add_subdirectory(deadcode)
