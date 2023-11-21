# Create the executable from all source files.
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

# Specify where to find the header files to include.
target_include_directories(CircuitSim2 PRIVATE .)

# Link to libraries and set C++ compilation version.
target_link_libraries(CircuitSim2 PRIVATE sfml-graphics portable_file_dialogs)
set_target_properties(CircuitSim2 PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Enable strict warnings. Could also use CMake presets here for better compiler support:
# https://alexreinking.com/blog/how-to-use-cmake-without-the-agonizing-pain-part-2.html
target_compile_options(CircuitSim2 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# need more here??? see sfml/cmake-sfml-project
# also see https://en.sfml-dev.org/forums/index.php?topic=28580.0


# Build the gui_lib.
add_subdirectory(gui)

add_executable(gui_demo
    gui/main2.cpp
)

target_link_libraries(gui_demo PRIVATE gui_lib sfml-graphics)
set_target_properties(gui_demo PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(gui_demo PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Build the new experimental app.
add_subdirectory(exp)
