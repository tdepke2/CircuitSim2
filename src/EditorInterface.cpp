#include <DebugScreen.h>
#include <Editor.h>
#include <EditorInterface.h>
#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>
#include <gui/Timer.h>
#include <gui/Widget.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/ChatBox.h>
#include <gui/widgets/DialogBox.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MenuBar.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/Panel.h>
#include <Locator.h>
#include <MakeUnique.h>
#include <MessageLogSink.h>
#include <ResourceBase.h>
#include <Tile.h>

#include <cassert>
#include <limits>
#include <spdlog/spdlog.h>
#include <string>

EditorInterface::EditorInterface(Editor& editor, sf::RenderWindow& window, MessageLogSinkMt* messageLogSink) :
    editor_(editor),
    debugWidgets_(),
    gui_(details::make_unique<gui::Gui>(window)),
    theme_(details::make_unique<gui::DefaultTheme>(*gui_, Locator::getResource()->getFont("resources/consolas.ttf"))) {

    auto menuBar = createMenuBar(window);
    gui_->addChild(menuBar);

    auto statusBar = debugWidgetCreation(gui::Panel::create(*theme_, "statusBar"));
    statusBar->setFocusable(false);
    gui_->addChild(statusBar);

    auto messageLog = debugWidgetCreation(gui::ChatBox::create(*theme_, "messageLog"));
    messageLog->setSizeCharacters({80, 12});
    messageLog->setMaxLines(500);
    messageLog->setAutoHide(true);
    messageLog->getStyle()->setFillColor({12, 12, 12});
    gui_->addChild(messageLog);

    // Register the message log with the spdlog sink so we can see debug log messages show up.
    if (messageLogSink != nullptr) {
        messageLogSink->registerChatBox(messageLog);
    }

    auto messageLogToggle = debugWidgetCreation(gui::Button::create(*theme_, "messageLogToggle"));
    messageLogToggle->setFocusable(false);
    messageLogToggle->setLabel("Messages (Ctrl+M)");
    messageLogToggle->setPosition(2.0f, 2.0f);
    messageLogToggle->onClick.connect([this]() {
        toggleMessageLog();
    });
    statusBar->addChild(messageLogToggle);

    cursorLabel_ = debugWidgetCreation(gui::Label::create(*theme_, "cursorLabel_"));
    cursorLabel_->setFocusable(false);
    // Initialize label text with the longest coords possible.
    cursorLabel_->setLabel("(" + std::to_string(std::numeric_limits<int>::min()) + ", " + std::to_string(std::numeric_limits<int>::min()) + ")");
    cursorLabel_->setVisible(false);
    statusBar->addChild(cursorLabel_);

    messageLogToggle->sendToFront();

    modalBackground_ = debugWidgetCreation(gui::Panel::create(*theme_, "modalBackground_"));
    modalBackground_->setVisible(false);
    modalBackground_->setFocusable(false);
    modalBackground_->getStyle()->setFillColor({0, 0, 0, 175});
    modalBackground_->getStyle()->setOutlineThickness(0.0f);
    gui_->addChild(modalBackground_);

    modalBackground_->addChild(createFileDialog());
    modalBackground_->addChild(createSaveDialog());
    modalBackground_->addChild(createOverwriteDialog());

    gui_->onWindowResized.connect([this,menuBar,statusBar,messageLog](gui::Gui* gui, sf::RenderWindow& /*window*/, const sf::Vector2u& size) {
        gui->setSize(size);
        menuBar->setWidth(static_cast<float>(size.x));
        statusBar->setSize({static_cast<float>(size.x), menuBar->getSize().y});
        statusBar->setPosition(0.0f, size.y - statusBar->getSize().y);
        messageLog->setSizeWithinBounds({static_cast<float>(size.x), messageLog->getSize().y});
        messageLog->setPosition(0.0f, statusBar->getPosition().y - messageLog->getSize().y);
        cursorLabel_->setPosition(statusBar->getSize().x - cursorLabel_->getSize().x, 2.0f);
        modalBackground_->setSize(static_cast<sf::Vector2f>(size));

        int menuBarHeight = static_cast<int>(menuBar->getSize().y);
        int statusBarHeight = static_cast<int>(statusBar->getSize().y);
        DebugScreen::instance()->setBorders({0, menuBarHeight + 0}, {0, statusBarHeight + 1});
    });

    // Trigger a fake event to initialize the size.
    sf::Event initSizeEvent;
    initSizeEvent.type = sf::Event::Resized;
    initSizeEvent.size.width = gui_->getSize().x;
    initSizeEvent.size.height = gui_->getSize().y;
    gui_->processEvent(initSizeEvent);
}

// Even if the dtor was just `~EditorInterface() = default`, we must declare it
// here instead of header file so that the unique_ptr does not need to know how
// to delete `gui_` in the header.
EditorInterface::~EditorInterface() {
    modalBackground_.reset();
    cursorLabel_.reset();
    theme_.reset();
    gui_.reset();

    unsigned int numLeakedWidgets = 0;
    for (const auto& widget : debugWidgets_) {
        if (!widget.second.expired()) {
            ++numLeakedWidgets;
        }
    }

    if (numLeakedWidgets > 0) {
        spdlog::warn("EditorInterface detected {} memory leaks out of {} registered widgets:", numLeakedWidgets, debugWidgets_.size());
        for (const auto& widget : debugWidgets_) {
            if (!widget.second.expired()) {
                spdlog::warn("  Widget \"{}\" not cleaned up! (ref count {}).", widget.first, widget.second.use_count());
            }
        }
    }
}

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

void EditorInterface::toggleMessageLog() {
    auto messageLog = gui_->getChild<gui::ChatBox>("messageLog");
    messageLog->setAutoHide(!messageLog->getAutoHide());
}

void EditorInterface::showFileDialog(bool openFile, const fs::path& filename) {
    modalBackground_->sendToFront();
    modalBackground_->setVisible(true);
    auto fileDialog = modalBackground_->getChild<gui::DialogBox>("fileDialog");
    auto fileDialogPtr = fileDialog.get();
    auto fileTitle = fileDialog->getChild<gui::Label>("fileTitle");
    auto fileTextBox = fileDialog->getChild<gui::MultilineTextBox>("fileTextBox");
    auto fileSubmitButton = fileDialog->getChild<gui::Button>("fileSubmitButton");

    if (openFile) {
        fileTitle->setLabel("Open File");
        fileTextBox->setText(filename.string());
        fileSubmitButton->setLabel("Open");
        fileSubmitButton->onClick.disconnectAll();
        fileSubmitButton->onClick.connect([this,fileDialogPtr,fileTextBox]() {
            fileDialogPtr->setVisible(false);
            modalBackground_->setVisible(false);
            if (!fileTextBox->getText().isEmpty()) {
                editor_.openBoard(true, fileTextBox->getText().toAnsiString());
            }
        });
    } else {
        fileTitle->setLabel("Save As");
        fileTextBox->setText(filename.string());
        fileSubmitButton->setLabel("Save");
        fileSubmitButton->onClick.disconnectAll();
        fileSubmitButton->onClick.connect([this,fileDialogPtr,fileTextBox]() {
            fileDialogPtr->setVisible(false);
            modalBackground_->setVisible(false);
            if (!fileTextBox->getText().isEmpty()) {
                editor_.saveAsBoard(fileTextBox->getText().toAnsiString());
            }
        });
    }

    // Force dialog to reposition widgets since labels changed.
    fileDialog->setSize(fileDialog->getSize());

    fileDialog->setVisible(true);
    fileDialog->setPosition(
        std::min(80.0f, modalBackground_->getSize().x / 2.0f),
        std::min(80.0f, modalBackground_->getSize().y / 2.0f)
    );
}

void EditorInterface::showSaveDialog(const std::function<void()>& action) {
    modalBackground_->sendToFront();
    modalBackground_->setVisible(true);
    auto saveDialog = modalBackground_->getChild<gui::DialogBox>("saveDialog");
    auto saveDialogPtr = saveDialog.get();

    auto saveSubmitButton = saveDialog->getChild<gui::Button>("saveSubmitButton");
    saveSubmitButton->onClick.disconnectAll();
    saveSubmitButton->onClick.connect([this,saveDialogPtr,action]() {
        saveDialogPtr->setVisible(false);
        modalBackground_->setVisible(false);
        editor_.saveBoard();
        action();    // FIXME: the action runs immediately and doesn't give gui time to update, do we want to run this in a Timer instance instead? Could that cause issues? (someone clears the timers, or we happen to run that in a different thread)
    });

    auto saveRefuseButton = saveDialog->getChild<gui::Button>("saveRefuseButton");
    saveRefuseButton->onClick.disconnectAll();
    saveRefuseButton->onClick.connect([this,saveDialogPtr,action]() {
        saveDialogPtr->setVisible(false);
        modalBackground_->setVisible(false);
        action();
    });

    saveDialog->setVisible(true);
    saveDialog->setPosition(
        std::min(80.0f, modalBackground_->getSize().x / 2.0f),
        std::min(80.0f, modalBackground_->getSize().y / 2.0f)
    );
}

void EditorInterface::showOverwriteDialog(const fs::path& filename) {
    modalBackground_->sendToFront();
    modalBackground_->setVisible(true);
    auto overwriteDialog = modalBackground_->getChild<gui::DialogBox>("overwriteDialog");
    auto overwriteDialogPtr = overwriteDialog.get();

    auto overwriteSubmitButton = overwriteDialog->getChild<gui::Button>("overwriteSubmitButton");
    overwriteSubmitButton->onClick.disconnectAll();
    overwriteSubmitButton->onClick.connect([this,overwriteDialogPtr,filename]() {
        overwriteDialogPtr->setVisible(false);
        modalBackground_->setVisible(false);
        editor_.saveAsBoard(filename, true);
    });

    overwriteDialog->setVisible(true);
    overwriteDialog->setPosition(
        std::min(80.0f, modalBackground_->getSize().x / 2.0f),
        std::min(80.0f, modalBackground_->getSize().y / 2.0f)
    );
}

bool EditorInterface::isModalDialogOpen() {
    return modalBackground_->isVisible();
}

bool EditorInterface::processEvent(const sf::Event& event) {
    return gui_->processEvent(event);
}

void EditorInterface::update() {
    gui::Timer::updateTimers();
}

template<typename T>
std::shared_ptr<T> EditorInterface::debugWidgetCreation(std::shared_ptr<T> widget) const {
    if (spdlog::get_level() > spdlog::level::debug) {    // FIXME: probably not ideal to check this way. check the cmake build type instead? if so, then set spdlog::level with that too.
        return widget;
    }

    const auto foundWidget = debugWidgets_.find(widget->getName());
    if (foundWidget != debugWidgets_.end() && !foundWidget->second.expired()) {
        spdlog::warn("Created widget with name \"{}\" which already exists.", foundWidget->first);
    }

    debugWidgets_[widget->getName()] = std::dynamic_pointer_cast<gui::Widget>(widget);
    return widget;
}

std::shared_ptr<gui::MenuBar> EditorInterface::createMenuBar(sf::RenderWindow& window) const {
    auto menuBar = debugWidgetCreation(gui::MenuBar::create(*theme_, "menuBar"));
    menuBar->setPosition(0.0f, 0.0f);

    gui::MenuList fileMenu("File");
    fileMenu.items.emplace_back("New", "Ctrl+N");
    fileMenu.items.emplace_back("Open...", "Ctrl+O");
    fileMenu.items.emplace_back("Save", "Ctrl+S");
    fileMenu.items.emplace_back("Save As...", "Ctrl+Shift+S");
    fileMenu.items.emplace_back("Rename...");
    fileMenu.items.emplace_back("Resize...");
    fileMenu.items.emplace_back("Configuration...");
    fileMenu.items.emplace_back("Exit");
    menuBar->insertMenu(fileMenu);

    auto chooseFileMenu = [this,&window](const sf::String& item) {
        if (item == "New") {
            editor_.newBoard();
        } else if (item == "Open...") {
            editor_.openBoard();
        } else if (item == "Save") {
            editor_.saveBoard();
        } else if (item == "Save As...") {
            editor_.saveAsBoard();
        } else if (item == "Rename...") {
            editor_.renameBoard();
        } else if (item == "Resize...") {
            editor_.resizeBoard();
        } else if (item == "Configuration...") {

        } else if (item == "Exit") {
            editor_.windowCloseRequested([&window]() {
                window.close();
            });
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

    auto menuBarPtr = menuBar.get();
    menuBar->onMenuItemClick.connect(
        [menuBarPtr,chooseFileMenu,chooseEditMenu,chooseViewMenu,chooseRunMenu,chooseToolsMenu,chooseWireMenu,chooseGateMenu,chooseMiscMenu]
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
        menuBarPtr->setFocused(false);
    });

    menuBar->onClick.connect([menuBarPtr]() {
        if (!menuBarPtr->isMenuOpen()) {
            menuBarPtr->setFocused(false);
        }
    });

    return menuBar;
}

std::shared_ptr<gui::DialogBox> EditorInterface::createFileDialog() const {
    auto fileDialog = debugWidgetCreation(gui::DialogBox::create(*theme_, "fileDialog"));
    fileDialog->setSize({520.0f, 100.0f});
    fileDialog->setVisible(false);
    auto fileDialogPtr = fileDialog.get();

    auto fileTitle = debugWidgetCreation(gui::Label::create(*theme_, "fileTitle"));
    fileDialog->setTitle(fileTitle);

    auto fileTextBox = debugWidgetCreation(gui::MultilineTextBox::create(*theme_, "fileTextBox"));
    fileTextBox->setSizeCharacters({60, 1});
    fileTextBox->setMaxLines(1);
    fileTextBox->setTabPolicy(gui::MultilineTextBox::TabPolicy::ignoreTab);
    fileTextBox->setPosition(10.0f, 30.0f);
    fileDialog->addChild(fileTextBox);

    auto fileSubmitButton = debugWidgetCreation(gui::Button::create(*theme_, "fileSubmitButton"));
    fileDialog->setSubmitButton(0, fileSubmitButton);

    auto fileCancelButton = debugWidgetCreation(gui::Button::create(*theme_, "fileCancelButton"));
    fileCancelButton->setLabel("Cancel");
    fileCancelButton->onClick.connect([this,fileDialogPtr]() {
        fileDialogPtr->setVisible(false);
        modalBackground_->setVisible(false);
    });
    fileDialog->setCancelButton(1, fileCancelButton);

    return fileDialog;
}

std::shared_ptr<gui::DialogBox> EditorInterface::createSaveDialog() const {
    auto saveDialog = debugWidgetCreation(gui::DialogBox::create(*theme_, "saveDialog"));
    saveDialog->setSize({300.0f, 100.0f});
    saveDialog->setVisible(false);
    auto saveDialogPtr = saveDialog.get();

    auto saveTitle = debugWidgetCreation(gui::Label::create(*theme_, "saveTitle"));
    saveTitle->setLabel("Save Changes?");
    saveDialog->setTitle(saveTitle);

    auto saveLabel = debugWidgetCreation(gui::Label::create(*theme_, "saveLabel"));
    saveLabel->setLabel("Board has been modified.");
    saveLabel->setPosition(10.0f, 30.0f);
    saveDialog->addChild(saveLabel);

    auto saveSubmitButton = debugWidgetCreation(gui::Button::create(*theme_, "saveSubmitButton"));
    saveSubmitButton->setLabel("Save");
    saveDialog->setSubmitButton(0, saveSubmitButton);

    auto saveRefuseButton = debugWidgetCreation(gui::Button::create(*theme_, "saveRefuseButton"));
    saveRefuseButton->setLabel("Don\'t Save");
    saveDialog->setOptionButton(1, saveRefuseButton);

    auto saveCancelButton = debugWidgetCreation(gui::Button::create(*theme_, "saveCancelButton"));
    saveCancelButton->setLabel("Cancel");
    saveCancelButton->onClick.connect([this,saveDialogPtr]() {
        saveDialogPtr->setVisible(false);
        modalBackground_->setVisible(false);
    });
    saveDialog->setCancelButton(2, saveCancelButton);

    return saveDialog;
}

std::shared_ptr<gui::DialogBox> EditorInterface::createOverwriteDialog() const {
    auto overwriteDialog = debugWidgetCreation(gui::DialogBox::create(*theme_, "overwriteDialog"));
    overwriteDialog->setSize({450.0f, 100.0f});
    overwriteDialog->setVisible(false);
    auto overwriteDialogPtr = overwriteDialog.get();

    auto overwriteTitle = debugWidgetCreation(gui::Label::create(*theme_, "overwriteTitle"));
    overwriteTitle->setLabel("Overwrite File?");
    overwriteDialog->setTitle(overwriteTitle);

    auto overwriteLabel = debugWidgetCreation(gui::Label::create(*theme_, "overwriteLabel"));
    overwriteLabel->setLabel("The file already exists, do you want to replace it?");
    overwriteLabel->setPosition(10.0f, 30.0f);
    overwriteDialog->addChild(overwriteLabel);

    auto overwriteSubmitButton = debugWidgetCreation(gui::Button::create(*theme_, "overwriteSubmitButton"));
    overwriteSubmitButton->setLabel("Overwrite");
    overwriteDialog->setSubmitButton(0, overwriteSubmitButton);

    auto overwriteCancelButton = debugWidgetCreation(gui::Button::create(*theme_, "overwriteCancelButton"));
    overwriteCancelButton->setLabel("Cancel");
    overwriteCancelButton->onClick.connect([this,overwriteDialogPtr]() {
        overwriteDialogPtr->setVisible(false);
        modalBackground_->setVisible(false);
    });
    overwriteDialog->setCancelButton(1, overwriteCancelButton);

    return overwriteDialog;
}

void EditorInterface::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(*gui_, states);
}
