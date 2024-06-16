#pragma once

#include <EditorInterface.h>
#include <OffsetView.h>
#include <SubBoard.h>
#include <Tile.h>
#include <TilePool.h>

#include <deque>
#include <memory>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <utility>

class Board;
class Command;

template<typename Mutex>
class MessageLogSink;
using MessageLogSinkMt = MessageLogSink<std::mutex>;

class Editor : public sf::Drawable {
public:
    Editor(Board& board, sf::RenderWindow& window, MessageLogSinkMt* messageLogSink);
    ~Editor() = default;
    Editor(const Editor& rhs) = delete;
    Editor& operator=(const Editor& rhs) = delete;

    const sf::Drawable& getGraphicalInterface() const;
    const OffsetView& getEditView() const;
    float getZoom() const;
    void goToTile(int x, int y);
    // Returns true if event was consumed (and should not be processed further).
    bool processEvent(const sf::Event& event);
    void update();

private:
    struct StaticInit {
        StaticInit();
        sf::Texture* tilesetBright;
        sf::Texture* tilesetBrightNoBlanks;
    };
    static StaticInit* staticInit_;

    enum class CursorState {
        empty, pickTile, pasteArea, wireTool
    };

    bool handleTextEntered(uint32_t unicode);
    bool handleKeyPressed(const sf::Event::KeyEvent& key);

    // FIXME: the following should not get bound to gui callbacks, instead have a shared callback that checks for the corresponding menu item.
    void undoEdit();
    void redoEdit();
    void selectAll();
    void deselectAll();
    void rotateTile(bool clockwise);
    void flipTile(bool acrossVertical);
    void editTile(bool toggleState);
    void copyArea();
    void pasteArea();
    void deleteArea();
    void wireTool();
    void queryTool();
    void pickTile(TileId::t id);

    void setCursorState(CursorState state);
    OffsetView findTileView(int x, int y) const;
    OffsetView findTileView(const sf::Vector2i& pos) const;
    std::pair<sf::Vector2i, bool> mapMouseToNearestTile(const sf::Vector2i& mousePos) const;
    void updateCursor();
    void updateSelection(const sf::Vector2i& newSelectionEnd);
    void highlightArea(sf::Vector2i a, sf::Vector2i b, bool highlight);
    template<typename T, typename... Args>
    std::unique_ptr<T> makeCommand(Args&&... args);
    void executeCommand(std::unique_ptr<Command>&& command);
    void pasteToBoard(const sf::Vector2i& tilePos, bool deltaCheck);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    EditorInterface interface_;
    Board& board_;
    OffsetView editView_;
    float zoomLevel_;
    sf::Vector2i mousePos_;
    bool mouseOnScreen_;
    sf::Vector2u windowSize_;

    sf::RectangleShape cursor_;
    std::pair<sf::Vector2i, bool> cursorCoords_;
    sf::Text cursorLabel_;
    CursorState cursorState_;
    bool cursorVisible_;
    std::pair<sf::Vector2i, bool> selectionStart_;
    sf::Vector2i selectionEnd_;
    SubBoard tileSubBoard_, copySubBoard_;

    TilePool tilePool_;
    std::deque<std::unique_ptr<Command>> editHistory_;
    size_t lastEditSize_;
};
