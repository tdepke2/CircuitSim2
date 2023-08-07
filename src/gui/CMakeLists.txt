set(HEADER_LIST
    Container.h
    ContainerBase.h
    Gui.h
    Signal.h
    Theme.h
    themes/DefaultTheme.h
    Widget.h
    widgets/Button.h
    widgets/MenuBar.h
    widgets/Panel.h
    widgets/TextBox.h
)

add_library(gui_lib
    Container.cpp
    ContainerBase.cpp
    Gui.cpp
    Signal.cpp
    Theme.cpp
    themes/DefaultTheme.cpp
    Widget.cpp
    widgets/Button.cpp
    widgets/MenuBar.cpp
    widgets/Panel.cpp
    widgets/TextBox.cpp
    ${HEADER_LIST}
)

target_include_directories(gui_lib PUBLIC .)
target_link_libraries(gui_lib PRIVATE sfml-graphics)
target_compile_features(gui_lib PRIVATE cxx_std_11)
set_target_properties(gui_lib PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(gui_lib PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
