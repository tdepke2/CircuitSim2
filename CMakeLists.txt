cmake_minimum_required(VERSION 3.14...3.26)

# Set project name and version.
project(CircuitSim2
    VERSION 1.1
    DESCRIPTION "Graphical tool for developing and testing digital logic circuits from the gate level"
    LANGUAGES CXX
)

# Check if this is the main project (not included with add_subdirectory).
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Enable support for folders in IDEs.
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Have CMake create a "compile_commands.json" file for clangd.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # SFML settings.
    set(BUILD_SHARED_LIBS OFF)
    set(SFML_BUILD_DOC OFF)
    set(SFML_BUILD_EXAMPLES OFF)

    set(SFML_BUILD_AUDIO OFF)
    set(SFML_BUILD_GRAPHICS ON)
    set(SFML_BUILD_NETWORK OFF)
    set(SFML_BUILD_WINDOW ON)
endif()

# Add the executable code.
add_subdirectory(src)
