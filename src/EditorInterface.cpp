#include <EditorInterface.h>
#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>
#include <gui/Timer.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/ChatBox.h>
#include <gui/widgets/MenuBar.h>

#include <array>
#include <mutex>
#include <spdlog/details/null_mutex.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <utility>

namespace {

template<typename Mutex>
class MySink : public spdlog::sinks::base_sink<Mutex> {
public:
    MySink(std::shared_ptr<gui::ChatBox> chatBox) :
        chatBox_(chatBox) {

        styles_[spdlog::level::trace] = {sf::Color::White, sf::Text::Regular};
        styles_[spdlog::level::debug] = {sf::Color::Cyan, sf::Text::Regular};
        styles_[spdlog::level::info] = {sf::Color::Green, sf::Text::Regular};
        styles_[spdlog::level::warn] = {sf::Color::Yellow, sf::Text::Regular};
        styles_[spdlog::level::err] = {sf::Color::Red, sf::Text::Regular};
        styles_[spdlog::level::critical] = {sf::Color::Red, sf::Text::Italic};
        styles_[spdlog::level::off] = {sf::Color::White, sf::Text::Regular};
    }

protected:
    virtual void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        chatBox_->addLines(fmt::to_string(formatted), styles_[msg.level].first, styles_[msg.level].second);
    }
    virtual void flush_() override {

    }

private:
    std::shared_ptr<gui::ChatBox> chatBox_;
    std::array<std::pair<sf::Color, uint32_t>, spdlog::level::n_levels> styles_;
};

using MySinkMt = MySink<std::mutex>;
using MySinkSt = MySink<spdlog::details::null_mutex>;

}

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

    auto messageLog = gui::ChatBox::create(*theme_, "messageLog");
    messageLog->setSizeCharacters({80, 15});
    messageLog->setMaxLines(100);
    messageLog->setAutoHide(true);
    messageLog->getStyle()->setFillColor({12, 12, 12});
    gui_->addChild(messageLog);

    auto sink = std::make_shared<MySinkMt>(messageLog);
    spdlog::default_logger()->sinks().push_back(sink);

    auto messageLogToggle = gui::Button::create(*theme_, "messageLogToggle");
    messageLogToggle->setLabel("^^^");
    messageLogToggle->setSize({messageLog->getSize().x, 20.0f});
    auto messageLogPtr = messageLog.get();
    messageLogToggle->onClick.connect([messageLogPtr]() {
        messageLogPtr->setAutoHide(!messageLogPtr->getAutoHide());
    });
    gui_->addChild(messageLogToggle);

    gui_->onWindowResized.connect([menuBar,messageLog,messageLogToggle](gui::Gui* gui, sf::RenderWindow& /*window*/, const sf::Vector2u& size) {
        gui->setSize(size);
        menuBar->setWidth(static_cast<float>(size.x));
        messageLogToggle->setPosition(0.0f, size.y - messageLogToggle->getSize().y);
        messageLog->setPosition(0.0f, messageLogToggle->getPosition().y - messageLog->getSize().y);
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
EditorInterface::~EditorInterface() {
    // FIXME: bit of a hack, need a better method.
    spdlog::default_logger()->sinks().pop_back();
}

bool EditorInterface::processEvent(const sf::Event& event) {
    return gui_->processEvent(event);
}

void EditorInterface::update() {
    gui::Timer::updateTimers();
}

void EditorInterface::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(*gui_, states);
}
