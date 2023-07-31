include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

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

target_link_libraries(CircuitSim2 PRIVATE sfml-graphics)
target_compile_features(CircuitSim2 PRIVATE cxx_std_11)
set_target_properties(CircuitSim2 PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(CircuitSim2 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# need more here??? see sfml/cmake-sfml-project
# also see https://en.sfml-dev.org/forums/index.php?topic=28580.0


add_executable(gui_demo
    gui/Button.cpp
    gui/Container.cpp
    gui/Gui.cpp
    gui/MenuBar.cpp
    gui/Panel.cpp
    gui/Theme.cpp
    gui/Widget.cpp
    gui/TextBox.cpp
    gui/themes/DefaultTheme.cpp
    gui/Signal.cpp
    gui/main2.cpp
)
target_include_directories(gui_demo PRIVATE .)
target_link_libraries(gui_demo PRIVATE sfml-graphics)
target_compile_features(gui_demo PRIVATE cxx_std_11)
set_target_properties(gui_demo PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(gui_demo PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
