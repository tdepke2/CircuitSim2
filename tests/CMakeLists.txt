# Import catch2 for unit testing.
message(STATUS "Fetching sources for catch2...")
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.10
)
FetchContent_MakeAvailable(catch2)

add_executable(FlatMap_test
    FlatMap.test.cpp
)
target_include_directories(FlatMap_test PRIVATE .)
target_link_libraries(FlatMap_test PRIVATE cs2_src_lib)
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

add_executable(mytest
    mytest.cpp
)
target_include_directories(mytest PRIVATE .)
target_link_libraries(mytest PRIVATE Catch2::Catch2)
set_target_properties(mytest PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
target_compile_options(mytest PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_test(NAME mytest1 COMMAND mytest)

add_subdirectory(gui)
