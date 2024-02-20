# Import catch2 for unit testing.
message(STATUS "Fetching sources for catch2...")
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.10
)
FetchContent_MakeAvailable(catch2)
message("")

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
include(Catch)

add_executable(cs2_src_test
    CatchMain.cpp
    FlatMap.test.cpp
    TilePool.test.cpp
)
target_link_libraries(cs2_src_test PRIVATE
    cs2_src
    Catch2::Catch2
)
cs2_add_cxx_properties(cs2_src_test)

catch_discover_tests(cs2_src_test)

add_subdirectory(gui)
