#include <EditorInterface.h>
#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>
#include <gui/widgets/MenuBar.h>

#include <spdlog/spdlog.h>

EditorInterface::EditorInterface(sf::RenderWindow& window) :
    gui_(new gui::Gui(window)),
    theme_(new gui::DefaultTheme(*gui_)) {    // FIXME: we should pass the existing font into the theme?

    auto menuBar = gui::MenuBar::create(*theme_, "menuBar");
    menuBar->setPosition(0.0f, 0.0f);

    gui::MenuList fileMenu("File");
    fileMenu.items.emplace_back("New", "Ctrl+N");
    fileMenu.items.emplace_back("Open...", "Ctrl+O");
    fileMenu.items.emplace_back("Save", "Ctrl+S");
    fileMenu.items.emplace_back("Save As...");
    fileMenu.items.emplace_back("Rename...");
    fileMenu.items.emplace_back("Resize...");
    fileMenu.items.emplace_back("Configuration...");
    fileMenu.items.emplace_back("Exit");
    menuBar->insertMenu(fileMenu);

    gui::MenuList editMenu("Edit");
    editMenu.items.emplace_back("Undo", "Ctrl+Z");
    editMenu.items.emplace_back("Redo", "Ctrl+Y");
    editMenu.items.emplace_back("Cut", "Ctrl+X");
    editMenu.items.emplace_back("Copy", "Ctrl+C");
    editMenu.items.emplace_back("Paste", "Ctrl+V");
    editMenu.items.emplace_back("  (Shift+Right Click = Force)", "", false);
    editMenu.items.emplace_back("  (Ctrl+Right Click = Skip Blanks)", "", false);
    editMenu.items.emplace_back("Delete", "DEL");
    menuBar->insertMenu(editMenu);

    gui::MenuList viewMenu("View");
    viewMenu.items.emplace_back("Toggle View/Edit Mode", "Enter");
    viewMenu.items.emplace_back("Zoom In", "Mouse Wheel Up");
    viewMenu.items.emplace_back("Zoom Out", "Mouse Wheel Down");
    viewMenu.items.emplace_back("Default Zoom");
    menuBar->insertMenu(viewMenu);

    gui::MenuList runMenu("Run");
    runMenu.items.emplace_back("Step One Tick", "Tab");
    runMenu.items.emplace_back("Change Max TPS", "Shift+Tab");
    menuBar->insertMenu(runMenu);

    gui::MenuList toolsMenu("Tools");
    toolsMenu.items.emplace_back("Select All", "Ctrl+A");
    toolsMenu.items.emplace_back("Deselect All", "ESC");
    toolsMenu.items.emplace_back("Rotate CW", "R");
    toolsMenu.items.emplace_back("Rotate CCW", "Shift+R");
    toolsMenu.items.emplace_back("Flip Across Vertical", "F");
    toolsMenu.items.emplace_back("Flip Across Horizontal", "Shift+F");
    toolsMenu.items.emplace_back("Toggle State", "E");
    toolsMenu.items.emplace_back("Edit/Alternative Tile", "Shift+E");
    toolsMenu.items.emplace_back("Wire Tool", "W");
    toolsMenu.items.emplace_back("Selection Query", "Q");
    menuBar->insertMenu(toolsMenu);

    gui::MenuList wireMenu("Wire");
    wireMenu.items.emplace_back("Blank (Eraser)", "Space");
    wireMenu.items.emplace_back("Straight", "T");
    wireMenu.items.emplace_back("Corner", "C");
    wireMenu.items.emplace_back("Tee", "Shift+T");
    wireMenu.items.emplace_back("Junction", "J");
    wireMenu.items.emplace_back("Crossover", "Shift+C");
    menuBar->insertMenu(wireMenu);

    gui::MenuList gateMenu("Gate");
    gateMenu.items.emplace_back("Diode", "D");
    gateMenu.items.emplace_back("Buffer", "B");
    gateMenu.items.emplace_back("NOT (Inverter)", "Shift+B");
    gateMenu.items.emplace_back("AND", "A");
    gateMenu.items.emplace_back("NAND", "Shift+A");
    gateMenu.items.emplace_back("OR", "O");
    gateMenu.items.emplace_back("NOR", "Shift+O");
    gateMenu.items.emplace_back("XOR", "X");
    gateMenu.items.emplace_back("XNOR", "Shift+X");
    menuBar->insertMenu(gateMenu);

    gui::MenuList miscMenu("Misc");
    miscMenu.items.emplace_back("Switch", "S");
    miscMenu.items.emplace_back("Button", "Shift+S");
    miscMenu.items.emplace_back("LED", "L");
    menuBar->insertMenu(miscMenu);

    gui_->addChild(menuBar);

    gui_->onWindowResized.connect([=](gui::Gui* gui, sf::RenderWindow& /*window*/, const sf::Vector2u& size) {
        gui->setSize(size);
        menuBar->setWidth(static_cast<float>(size.x));
    });

    // Trigger a fake event to initialize the size.
    sf::Event initSizeEvent;
    initSizeEvent.type = sf::Event::Resized;
    initSizeEvent.size.width = gui_->getSize().x;
    initSizeEvent.size.height = gui_->getSize().y;
    gui_->processEvent(initSizeEvent);
}

// Must declare the dtor here instead of header file so that the unique_ptr does
// not need to know how to delete `gui_` in the header.
EditorInterface::~EditorInterface() = default;

bool EditorInterface::processEvent(const sf::Event& event) {
    return gui_->processEvent(event);
}

void EditorInterface::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(*gui_, states);
}
