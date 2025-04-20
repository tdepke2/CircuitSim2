#pragma once

#include <Filesystem.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <string>

namespace gui {
    class DialogBox;
    class Gui;
    class Label;
    class MenuBar;
    class Panel;
    class Theme;
    class Widget;
}

class Editor;

template<typename Mutex>
class MessageLogSink;
using MessageLogSinkMt = MessageLogSink<std::mutex>;

class EditorInterface : public sf::Drawable {
public:
    EditorInterface(Editor& editor, sf::RenderWindow& window, MessageLogSinkMt* messageLogSink);
    ~EditorInterface();
    EditorInterface(const EditorInterface& rhs) = delete;
    EditorInterface& operator=(const EditorInterface& rhs) = delete;

    void setCursorVisible(bool visible);
    void updateCursorCoords(const sf::Vector2i& coords);
    void toggleMessageLog();
    void showFileDialog(bool openFile, const fs::path& filename);
    void showSaveDialog(const std::function<void()>& action);
    void showOverwriteDialog(const fs::path& filename);
    bool isModalDialogOpen();

    // Returns true if event was consumed (and should not be processed further).
    bool processEvent(const sf::Event& event);
    void update();

private:
    template<typename T>
    std::shared_ptr<T> debugWidgetCreation(std::shared_ptr<T> widget) const;
    std::shared_ptr<gui::MenuBar> createMenuBar(sf::RenderWindow& window) const;
    std::shared_ptr<gui::DialogBox> createFileDialog() const;
    std::shared_ptr<gui::DialogBox> createSaveDialog() const;
    std::shared_ptr<gui::DialogBox> createOverwriteDialog() const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    Editor& editor_;
    mutable std::map<std::string, std::weak_ptr<gui::Widget>> debugWidgets_;
    std::unique_ptr<gui::Gui> gui_;
    std::unique_ptr<gui::Theme> theme_;
    std::shared_ptr<gui::Label> cursorLabel_;
    std::shared_ptr<gui::Panel> modalBackground_;
};
