add_executable(CircuitSim2
    Board.cpp
    Simulator.cpp
    Tile.cpp
    TileButton.cpp
    TileGate.cpp
    TileLED.cpp
    TileSwitch.cpp
    TileWire.cpp
    UserInterface.cpp
    main.cpp
)

target_include_directories(CircuitSim2 PRIVATE .)

target_link_libraries(CircuitSim2 PRIVATE
    sfml-graphics
    portable_file_dialogs
)
set_target_properties(CircuitSim2 PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(CircuitSim2 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
