#pragma once

#include <memory>
#include <mutex>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Label;
    class MenuBar;
    class Theme;
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

    // Returns true if event was consumed (and should not be processed further).
    bool processEvent(const sf::Event& event);
    void update();

private:
    std::shared_ptr<gui::MenuBar> createMenuBar() const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    Editor& editor_;
    std::unique_ptr<gui::Gui> gui_;
    std::unique_ptr<gui::Theme> theme_;
    std::shared_ptr<gui::Label> cursorLabel_;
};
