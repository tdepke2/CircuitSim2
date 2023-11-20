cmake_minimum_required(VERSION 3.14...3.26)

# Set project name and version.
project(CircuitSim2
    VERSION 1.1
    DESCRIPTION "Graphical tool for developing and testing digital logic circuits from the gate level"
    LANGUAGES CXX
)

# Check if this is the main project (not included with add_subdirectory).
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Set default build type if unspecified.
    # From https://cliutils.gitlab.io/modern-cmake/chapters/features.html
    set(default_build_type "Release")
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
        set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE
        )
        # Set the possible values of build type for cmake-gui
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
        )
    elseif(CMAKE_BUILD_TYPE)
        message(STATUS "Current build type is '${CMAKE_BUILD_TYPE}'.")
    elseif(CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Build type is not set, select it during build with '--config' option and one of the following: '${CMAKE_CONFIGURATION_TYPES}'.")
    endif()

    # Place binaries in lib/ or bin/ respectively instead of in the sources directory.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    # Have CMake create a "compile_commands.json" file for clangd.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Enable support for folders in IDEs.
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # SFML settings.
    set(BUILD_SHARED_LIBS OFF)
    set(SFML_BUILD_DOC OFF)
    set(SFML_BUILD_EXAMPLES OFF)

    set(SFML_BUILD_AUDIO OFF)
    set(SFML_BUILD_GRAPHICS ON)
    set(SFML_BUILD_NETWORK OFF)
    set(SFML_BUILD_WINDOW ON)

    set(WARNINGS_AS_ERRORS OFF)
endif()

include(FetchContent)
#set(FETCHCONTENT_QUIET OFF)

# Import the SFML library.
message(STATUS "Fetching sources for SFML...")
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

# Import portable-file-dialogs for native system file dialogs.
# Some other options were considered too and may be relevant in future:
# https://sourceforge.net/projects/tinyfiledialogs/
# https://github.com/btzy/nativefiledialog-extended
message(STATUS "Fetching sources for portable_file_dialogs...")
FetchContent_Declare(
    portable_file_dialogs
    GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git
    GIT_TAG 7f852d88a480020d7f91957cbcefe514fc95000c    # Latest commit as of writing.
)
FetchContent_MakeAvailable(portable_file_dialogs)

# Import spdlog for logging utilities.
message(STATUS "Fetching sources for spdlog...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Import features from c++17 std::filesystem.
message(STATUS "Fetching sources for ghc_filesystem...")
FetchContent_Declare(
    ghc_filesystem
    GIT_REPOSITORY https://github.com/gulrak/filesystem.git
    GIT_TAG v1.5.14
)
FetchContent_MakeAvailable(ghc_filesystem)

# Run the next CMakeLists file (builds the executable).
add_subdirectory(src)
