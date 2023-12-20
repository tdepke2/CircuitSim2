add_executable(CircuitSim2_experimental
    Board.cpp
    Chunk.cpp
    ChunkDrawable.cpp
    ChunkGroup.cpp
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
    Tile.cpp
    tiles/Blank.cpp
    tiles/Gate.cpp
    tiles/Input.cpp
    tiles/Led.cpp
    tiles/Wire.cpp
    TileType.cpp
)

target_include_directories(CircuitSim2_experimental PRIVATE .)

target_link_libraries(CircuitSim2_experimental PRIVATE sfml-graphics spdlog::spdlog ghc_filesystem)
set_target_properties(CircuitSim2_experimental PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_compile_options(CircuitSim2_experimental PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_executable(FlatMap_test
    FlatMap.test.cpp
)
target_include_directories(FlatMap_test PRIVATE .)
set_target_properties(FlatMap_test PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
target_compile_options(FlatMap_test PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
