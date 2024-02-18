# Import catch2 for unit testing.
message(STATUS "Fetching sources for catch2...")
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.10
)
FetchContent_MakeAvailable(catch2)
message("")

add_executable(FlatMap_test
    FlatMap.test.cpp
)
target_link_libraries(FlatMap_test PRIVATE cs2_src)
cs2_add_cxx_properties(FlatMap_test)

add_executable(mytest
    mytest.cpp
)
target_link_libraries(mytest PRIVATE Catch2::Catch2)
cs2_add_cxx_properties(mytest)

add_test(NAME mytest1 COMMAND mytest)

add_subdirectory(gui)
