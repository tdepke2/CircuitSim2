add_executable(CircuitSim2_experimental
    Board.cpp
    Chunk.cpp
    ChunkDrawable.cpp
    ChunkRender.cpp
    DebugScreen.cpp
    FileStorage.cpp
    Filesystem.cpp
    LegacyFileFormat.cpp
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
target_compile_features(CircuitSim2_experimental PRIVATE cxx_std_11)
set_target_properties(CircuitSim2_experimental PROPERTIES
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
target_compile_features(FlatMap_test PRIVATE cxx_std_11)
set_target_properties(FlatMap_test PROPERTIES
    CXX_EXTENSIONS OFF
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
target_compile_options(FlatMap_test PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
