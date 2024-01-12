#pragma once

#include <OffsetView.h>
#include <SubBoard.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <utility>

class Board;
class ResourceManager;

class Editor : public sf::Drawable {
public:
    static void setupTextureData(sf::Texture* tilesetGrid, unsigned int tileWidth);

    Editor(Board& board, ResourceManager& resource, const sf::View& initialView);
    ~Editor();
    Editor(const Editor& rhs) = delete;
    Editor& operator=(const Editor& rhs) = delete;

    const OffsetView& getEditView() const;
    float getZoom() const;
    void processEvent(const sf::Event& event);
    void update();

private:
    static std::unique_ptr<sf::Texture> tilesetBright_;
    static std::unique_ptr<sf::Texture> tilesetBrightNoBlanks_;
    static unsigned int tileWidth_;

    void deselectAll();

    sf::Vector2i mapMouseToTile(const sf::Vector2i& mousePos) const;
    void updateCursor();
    void updateSelection(const sf::Vector2i& newSelectionEnd);
    void highlightArea(sf::Vector2i a, sf::Vector2i b, bool highlight);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    Board& board_;
    OffsetView editView_;
    float zoomLevel_;
    sf::Vector2i mousePos_;
    bool mouseOnScreen_;
    sf::Vector2u windowSize_;

    sf::RectangleShape cursor_;
    sf::Vector2i cursorCoords_;
    sf::Text cursorLabel_;
    bool cursorVisible_;
    std::pair<sf::Vector2i, bool> selectionStart_;
    sf::Vector2i selectionEnd_;
    SubBoard subBoard_;
};
