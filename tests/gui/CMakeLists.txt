add_executable(gui_demo
    main2.cpp
)
target_link_libraries(gui_demo PRIVATE
    cs2_gui_lib
    sfml-graphics
)
set_target_properties(gui_demo PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
target_compile_options(gui_demo PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
