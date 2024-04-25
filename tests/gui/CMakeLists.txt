add_executable(gui_demo
    main.cpp
)
target_link_libraries(gui_demo PRIVATE
    cs2_gui
    sfml-graphics
)
cs2_add_cxx_properties(gui_demo)

add_executable(gui_test
    Gui.test.cpp
)
target_link_libraries(gui_test PRIVATE
    sfml-graphics
)
cs2_add_cxx_properties(gui_test)
