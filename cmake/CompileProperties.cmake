# Require (and enforce) C++11 standard.
# An alternative method is to put these in the target properties, but it's more
# flexible to allow the language standard to be changed for all targets with a
# single variable.
set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard to use.")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries.")

# Set a required c++ version and strict warnings for a target.
function(cs2_add_cxx_properties TARGET_NAME)
    # Target compile features sets the standard in a more flexible way, but we
    # want to specify a specific standard for compatibility.
    #target_compile_features(${TARGET_NAME} PUBLIC cxx_std_11)

    set_target_properties(${TARGET_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    # Enable strict warnings. Could also use CMake presets here for better compiler support:
    # https://alexreinking.com/blog/how-to-use-cmake-without-the-agonizing-pain-part-2.html
    target_compile_options(${TARGET_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )
endfunction()
