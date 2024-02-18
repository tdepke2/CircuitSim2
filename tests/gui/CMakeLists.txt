add_executable(gui_demo
    main2.cpp
)
target_link_libraries(gui_demo PRIVATE
    cs2_gui
    sfml-graphics
)
cs2_add_cxx_properties(gui_demo)
