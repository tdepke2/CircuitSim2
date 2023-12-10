#pragma once

#include <OffsetView.h>

#include <SFML/Graphics.hpp>

class Board;
class ResourceManager;

class Editor : public sf::Drawable {
public:
    static void setup(unsigned int tileWidth);

    Editor(Board& board, ResourceManager& resource);
    ~Editor() = default;
    Editor(const Editor& rhs) = delete;
    Editor& operator=(const Editor& rhs) = delete;

    void processEvent(const sf::Event& event);
    void update(const OffsetView& offsetView, float zoom);

private:
    static unsigned int tileWidth_;

    void updateCursor();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    Board& board_;
    OffsetView offsetView_;
    float zoomLevel_;
    sf::Vector2i mousePos_;
    sf::RectangleShape cursor_;
    sf::Vector2i cursorCoords_;
    sf::Text cursorLabel_;
};
