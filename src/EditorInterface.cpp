#include <Editor.h>
#include <EditorInterface.h>
#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>
#include <gui/Timer.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/ChatBox.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MenuBar.h>
#include <gui/widgets/Panel.h>
#include <Locator.h>
#include <MessageLogSink.h>
#include <ResourceBase.h>
#include <Tile.h>

#include <limits>
#include <spdlog/spdlog.h>
#include <string>

EditorInterface::EditorInterface(Editor& editor, sf::RenderWindow& window, MessageLogSinkMt* messageLogSink) :
    editor_(editor),
    gui_(new gui::Gui(window)),
    theme_(new gui::DefaultTheme(*gui_, Locator::getResource()->getFont("resources/consolas.ttf"))) {

    auto menuBar = createMenuBar();
    gui_->addChild(menuBar);

    auto statusBar = gui::Panel::create(*theme_, "statusBar");
    gui_->addChild(statusBar);

    // FIXME: have the message log stretch to fit the screen?
    // we also should have a keybind to open/close it. losing focus may also need to close it?

    auto messageLog = gui::ChatBox::create(*theme_, "messageLog");
    messageLog->setSizeCharacters({80, 15});
    messageLog->setMaxLines(500);
    messageLog->setAutoHide(true);
    messageLog->getStyle()->setFillColor({12, 12, 12});
    gui_->addChild(messageLog);

    // Register the message log with the spdlog sink so we can see debug log messages show up.
    if (messageLogSink != nullptr) {
        messageLogSink->registerChatBox(messageLog);
    }

    auto messageLogToggle = gui::Button::create(*theme_, "messageLogToggle");
    messageLogToggle->setLabel("Message Log");
    messageLogToggle->setPosition(2.0f, 2.0f);
    auto messageLogPtr = messageLog.get();
    messageLogToggle->onClick.connect([messageLogPtr]() {
        messageLogPtr->setAutoHide(!messageLogPtr->getAutoHide());
    });
    statusBar->addChild(messageLogToggle);

    cursorLabel_ = gui::Label::create(*theme_, "cursorLabel_");
    // Initialize label text with the longest coords possible.
    cursorLabel_->setLabel("(" + std::to_string(std::numeric_limits<int>::min()) + ", " + std::to_string(std::numeric_limits<int>::min()) + ")");
    cursorLabel_->setVisible(false);
    statusBar->addChild(cursorLabel_);

    gui_->onWindowResized.connect([this,menuBar,statusBar,messageLog](gui::Gui* gui, sf::RenderWindow& /*window*/, const sf::Vector2u& size) {
        gui->setSize(size);
        menuBar->setWidth(static_cast<float>(size.x));
        statusBar->setSize({static_cast<float>(size.x), menuBar->getSize().y});
        statusBar->setPosition(0.0f, size.y - statusBar->getSize().y);
        messageLog->setPosition(0.0f, statusBar->getPosition().y - messageLog->getSize().y);
        cursorLabel_->setPosition(statusBar->getSize().x - cursorLabel_->getSize().x, 2.0f);
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

void EditorInterface::setCursorVisible(bool visible) {
    cursorLabel_->setVisible(visible);
}

void EditorInterface::updateCursorCoords(const sf::Vector2i& coords) {
    std::string label = "(" + std::to_string(coords.x) + ", " + std::to_string(coords.y) + ")";
    if (cursorLabel_->getLabel().getSize() > label.size()) {
        cursorLabel_->setLabel(std::string(cursorLabel_->getLabel().getSize() - label.size(), '.') + label);
    } else {
        cursorLabel_->setLabel(label);
    }
}

bool EditorInterface::processEvent(const sf::Event& event) {
    return gui_->processEvent(event);
}

void EditorInterface::update() {
    gui::Timer::updateTimers();
}

std::shared_ptr<gui::MenuBar> EditorInterface::createMenuBar() const {
    // FIXME: need to resolve some issues with menu bar and other widgets event handling.
    // 1. menu bar eats mouse move events, but open message log doesn't?
    // 2. clicking menu option keeps focus on it, same for some other widgets. have to press esc an extra time to deselect something.
    // 3. clicking menu option does not pass a mouse move event to cursor, user moves cursor and sees it "jump". minor issue though.
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

    auto chooseFileMenu = [](const sf::String& item) {
        if (item == "New") {

        } else if (item == "Open...") {

        } else if (item == "Save") {

        } else if (item == "Save As...") {

        } else if (item == "Rename...") {

        } else if (item == "Resize...") {

        } else if (item == "Configuration...") {

        } else if (item == "Exit") {

        }
    };

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

    auto chooseEditMenu = [this](const sf::String& item) {
        if (item == "Undo") {
            editor_.undoEdit();
        } else if (item == "Redo") {
            editor_.redoEdit();
        } else if (item == "Cut") {
            editor_.copyArea();
            editor_.deleteArea();
        } else if (item == "Copy") {
            editor_.copyArea();
        } else if (item == "Paste") {
            editor_.pasteArea();
        } else if (item == "Delete") {
            editor_.deleteArea();
        }
    };

    gui::MenuList viewMenu("View");
    viewMenu.items.emplace_back("Toggle View/Edit Mode", "Enter");
    viewMenu.items.emplace_back("Zoom In", "Mouse Wheel Up");
    viewMenu.items.emplace_back("Zoom Out", "Mouse Wheel Down");
    viewMenu.items.emplace_back("Default Zoom");
    menuBar->insertMenu(viewMenu);

    auto chooseViewMenu = [](const sf::String& item) {
        if (item == "Toggle View/Edit Mode") {

        } else if (item == "Zoom In") {

        } else if (item == "Zoom Out") {

        } else if (item == "Default Zoom") {

        }
    };

    gui::MenuList runMenu("Run");
    runMenu.items.emplace_back("Step One Tick", "Tab");
    runMenu.items.emplace_back("Change Max TPS", "Shift+Tab");
    menuBar->insertMenu(runMenu);

    auto chooseRunMenu = [](const sf::String& item) {
        if (item == "Step One Tick") {

        } else if (item == "Change Max TPS") {

        }
    };

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

    auto chooseToolsMenu = [this](const sf::String& item) {
        if (item == "Select All") {
            editor_.selectAll();
        } else if (item == "Deselect All") {
            editor_.deselectAll();
        } else if (item == "Rotate CW") {
            editor_.rotateTile(true);
        } else if (item == "Rotate CCW") {
            editor_.rotateTile(false);
        } else if (item == "Flip Across Vertical") {
            editor_.flipTile(true);
        } else if (item == "Flip Across Horizontal") {
            editor_.flipTile(false);
        } else if (item == "Toggle State") {
            editor_.editTile(true);
        } else if (item == "Edit/Alternative Tile") {
            editor_.editTile(false);
        } else if (item == "Wire Tool") {
            editor_.wireTool();
        } else if (item == "Selection Query") {
            editor_.queryTool();
        }
    };

    gui::MenuList wireMenu("Wire");
    wireMenu.items.emplace_back("Blank (Eraser)", "Space");
    wireMenu.items.emplace_back("Straight", "T");
    wireMenu.items.emplace_back("Corner", "C");
    wireMenu.items.emplace_back("Tee", "Shift+T");
    wireMenu.items.emplace_back("Junction", "J");
    wireMenu.items.emplace_back("Crossover", "Shift+C");
    menuBar->insertMenu(wireMenu);

    auto chooseWireMenu = [this](const sf::String& item) {
        if (item == "Blank (Eraser)") {
            editor_.pickTile(TileId::blank);
        } else if (item == "Straight") {
            editor_.pickTile(TileId::wireStraight);
        } else if (item == "Corner") {
            editor_.pickTile(TileId::wireCorner);
        } else if (item == "Tee") {
            editor_.pickTile(TileId::wireTee);
        } else if (item == "Junction") {
            editor_.pickTile(TileId::wireJunction);
        } else if (item == "Crossover") {
            editor_.pickTile(TileId::wireCrossover);
        }
    };

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

    auto chooseGateMenu = [this](const sf::String& item) {
        if (item == "Diode") {
            editor_.pickTile(TileId::gateDiode);
        } else if (item == "Buffer") {
            editor_.pickTile(TileId::gateBuffer);
        } else if (item == "NOT (Inverter)") {
            editor_.pickTile(TileId::gateNot);
        } else if (item == "AND") {
            editor_.pickTile(TileId::gateAnd);
        } else if (item == "NAND") {
            editor_.pickTile(TileId::gateNand);
        } else if (item == "OR") {
            editor_.pickTile(TileId::gateOr);
        } else if (item == "NOR") {
            editor_.pickTile(TileId::gateNor);
        } else if (item == "XOR") {
            editor_.pickTile(TileId::gateXor);
        } else if (item == "XNOR") {
            editor_.pickTile(TileId::gateXnor);
        }
    };

    gui::MenuList miscMenu("Misc");
    miscMenu.items.emplace_back("Switch", "S");
    miscMenu.items.emplace_back("Button", "Shift+S");
    miscMenu.items.emplace_back("LED", "L");
    miscMenu.items.emplace_back("Label", "Shift+L");
    menuBar->insertMenu(miscMenu);

    auto chooseMiscMenu = [this](const sf::String& item) {
        if (item == "Switch") {
            editor_.pickTile(TileId::inSwitch);
        } else if (item == "Button") {
            editor_.pickTile(TileId::inButton);
        } else if (item == "LED") {
            editor_.pickTile(TileId::outLed);
        } else if (item == "Label") {
            editor_.pickTile(TileId::label);
        }
    };

    menuBar->onMenuItemClick.connect(
        [chooseFileMenu,chooseEditMenu,chooseViewMenu,chooseRunMenu,chooseToolsMenu,chooseWireMenu,chooseGateMenu,chooseMiscMenu]
        (gui::Widget* /*w*/, const gui::MenuList& menu, size_t index) {

        const auto& item = menu.items[index].leftText;
        if (menu.name == "File") {
            chooseFileMenu(item);
        } else if (menu.name == "Edit") {
            chooseEditMenu(item);
        } else if (menu.name == "View") {
            chooseViewMenu(item);
        } else if (menu.name == "Run") {
            chooseRunMenu(item);
        } else if (menu.name == "Tools") {
            chooseToolsMenu(item);
        } else if (menu.name == "Wire") {
            chooseWireMenu(item);
        } else if (menu.name == "Gate") {
            chooseGateMenu(item);
        } else if (menu.name == "Misc") {
            chooseMiscMenu(item);
        }
    });

    return menuBar;
}

void EditorInterface::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(*gui_, states);
}
