set(CS2_GUI_HEADERS
    Container.h
    ContainerBase.h
    Gui.h
    Signal.h
    Theme.h
    themes/DefaultTheme.h
    Widget.h
    widgets/Button.h
    widgets/CheckBox.h
    widgets/DialogBox.h
    widgets/Group.h
    widgets/Label.h
    widgets/MenuBar.h
    widgets/MultilineTextBox.h
    widgets/Panel.h
    widgets/RadioButton.h
    widgets/TextBox.h
)
list(TRANSFORM CS2_GUI_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/gui/")

add_library(cs2_gui
    Container.cpp
    ContainerBase.cpp
    Gui.cpp
    Signal.cpp
    Theme.cpp
    themes/DefaultTheme.cpp
    Widget.cpp
    widgets/Button.cpp
    widgets/CheckBox.cpp
    widgets/DialogBox.cpp
    widgets/Group.cpp
    widgets/Label.cpp
    widgets/MenuBar.cpp
    widgets/MultilineTextBox.cpp
    widgets/Panel.cpp
    widgets/RadioButton.cpp
    widgets/TextBox.cpp
    ${CS2_GUI_HEADERS}
)

source_group(
    TREE "${PROJECT_SOURCE_DIR}/include"
    PREFIX "Header Files"
    FILES ${CS2_GUI_HEADERS}
)

target_include_directories(cs2_gui PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(cs2_gui PUBLIC sfml-graphics)
cs2_add_cxx_properties(cs2_gui)
