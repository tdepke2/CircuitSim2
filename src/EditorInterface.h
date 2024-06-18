#pragma once

#include <memory>
#include <mutex>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Label;
    class Theme;
}

template<typename Mutex>
class MessageLogSink;
using MessageLogSinkMt = MessageLogSink<std::mutex>;

class EditorInterface : public sf::Drawable {
public:
    EditorInterface(sf::RenderWindow& window, MessageLogSinkMt* messageLogSink);
    ~EditorInterface();
    EditorInterface(const EditorInterface& rhs) = delete;
    EditorInterface& operator=(const EditorInterface& rhs) = delete;

    void setCursorVisible(bool visible);
    void updateCursorCoords(const sf::Vector2i& coords);

    // Returns true if event was consumed (and should not be processed further).
    bool processEvent(const sf::Event& event);
    void update();

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::unique_ptr<gui::Gui> gui_;
    std::unique_ptr<gui::Theme> theme_;
    std::shared_ptr<gui::Label> cursorLabel_;
};
