#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

class EditorInterface : public sf::Drawable {
public:
    EditorInterface(sf::RenderWindow& window);
    ~EditorInterface();
    EditorInterface(const EditorInterface& rhs) = delete;
    EditorInterface& operator=(const EditorInterface& rhs) = delete;

    // Returns true if event was consumed (and should not be processed further).
    bool processEvent(const sf::Event& event);
    void update();

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::unique_ptr<gui::Gui> gui_;
    std::unique_ptr<gui::Theme> theme_;
};
