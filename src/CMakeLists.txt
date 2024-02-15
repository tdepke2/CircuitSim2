set(HEADER_LIST
    Board.h
    Chunk.h
    ChunkDrawable.h
    ChunkRender.h
    DebugScreen.h
    Editor.h
    FileStorage.h
    Filesystem.h
    LegacyFileFormat.h
    LodRenderer.h
    OffsetView.h
    RegionFileFormat.h
    ResourceManager.h
    SubBoard.h
    Tile.h
    TilePool.h
    tiles/Blank.h
    tiles/Gate.h
    tiles/Input.h
    tiles/Led.h
    tiles/Wire.h
    TileType.h
)

add_library(cs2_src_lib
    Board.cpp
    Chunk.cpp
    ChunkDrawable.cpp
    ChunkRender.cpp
    DebugScreen.cpp
    Editor.cpp
    FileStorage.cpp
    Filesystem.cpp
    LegacyFileFormat.cpp
    LodRenderer.cpp
    main.cpp
    OffsetView.cpp
    RegionFileFormat.cpp
    ResourceManager.cpp
    SubBoard.cpp
    Tile.cpp
    TilePool.cpp
    tiles/Blank.cpp
    tiles/Gate.cpp
    tiles/Input.cpp
    tiles/Led.cpp
    tiles/Wire.cpp
    TileType.cpp
    ${HEADER_LIST}
)

target_include_directories(cs2_src_lib PUBLIC .)

target_link_libraries(cs2_src_lib PUBLIC
    sfml-graphics
    spdlog::spdlog
    ghc_filesystem
)
set_target_properties(cs2_src_lib PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(cs2_src_lib PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Create the executable from all source files.
add_executable(CircuitSim2_experimental
    main.cpp
)

# Specify where to find the header files to include.
target_include_directories(CircuitSim2_experimental PRIVATE .)

# Link to libraries and set C++ compilation version.
target_link_libraries(CircuitSim2_experimental PRIVATE
    cs2_src_lib
)
set_target_properties(CircuitSim2_experimental PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Enable strict warnings. Could also use CMake presets here for better compiler support:
# https://alexreinking.com/blog/how-to-use-cmake-without-the-agonizing-pain-part-2.html
target_compile_options(CircuitSim2_experimental PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# FIXME: need more here??? see sfml/cmake-sfml-project
# also see https://en.sfml-dev.org/forums/index.php?topic=28580.0

# Build the gui.
add_subdirectory(gui)

# Build the old (stable) app.
add_subdirectory(deadcode)
