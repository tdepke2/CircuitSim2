add_executable(CircuitSim2
    Board.cpp
    Board.h
    main.cpp
    Simulator.cpp
    Simulator.h
    Tile.cpp
    Tile.h
    TileButton.cpp
    TileButton.h
    TileGate.cpp
    TileGate.h
    TileLED.cpp
    TileLED.h
    TileSwitch.cpp
    TileSwitch.h
    TileWire.cpp
    TileWire.h
    UserInterface.cpp
    UserInterface.h
)

target_include_directories(CircuitSim2 PRIVATE .)
target_link_libraries(CircuitSim2 PRIVATE
    sfml-graphics
    portable_file_dialogs
)
cs2_add_cxx_properties(CircuitSim2)
