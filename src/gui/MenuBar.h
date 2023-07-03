#pragma once

#include <gui/Signal.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

struct MenuItem {
public:
    MenuItem(sf::String leftText = "", sf::String rightText = "", bool enabled = true) :
        leftText(leftText),
        rightText(rightText),
        enabled(enabled) {
    }

    sf::String leftText, rightText;
    bool enabled;

private:
    sf::Vector2f leftPosition_, rightPosition_;
};

struct MenuList {
public:
    MenuList(sf::String name = "") :
        name(name) {
    }

    sf::String name;
    std::vector<MenuItem> items;

private:
    sf::Vector2f labelPosition_, menuSize_;

    friend class MenuBar;
};

class MenuBarStyle {
public:
    MenuBarStyle(const Gui& gui);

    // sf::Shape interface.
    void setTexture(const sf::Texture* texture, bool resetRect = false);
    void setTextureRect(const sf::IntRect& rect);
    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);
    void setOutlineThickness(float thickness);
    const sf::Texture* getTexture() const;
    const sf::IntRect& getTextureRect() const;
    const sf::Color& getFillColor() const;
    const sf::Color& getOutlineColor() const;
    float getOutlineThickness() const;

    // sf::Text interface.
    void setFont(const sf::Font& font);
    void setCharacterSize(unsigned int size);
    void setLineSpacing(float spacingFactor);
    void setLetterSpacing(float spacingFactor);
    void setTextStyle(uint32_t style);
    void setTextFillColor(const sf::Color& color);
    const sf::Font* getFont() const;
    unsigned int getCharacterSize() const;
    float getLineSpacing() const;
    float getLetterSpacing() const;
    uint32_t getTextStyle() const;
    const sf::Color& getTextFillColor() const;

    void setTextPadding(const sf::Vector3f& padding);
    const sf::Vector3f& getTextPadding() const;

    std::shared_ptr<MenuBarStyle> clone() const;

private:
    const Gui& gui_;
    sf::RectangleShape menu_;   // FIXME add another one for the menu vs bar??? ##############################
    sf::Text text_;
    sf::Vector3f textPadding_;

    friend class MenuBar;
};

class MenuBar : public Widget {
public:
    static std::shared_ptr<MenuBar> create(const Theme& theme);
    static std::shared_ptr<MenuBar> create(std::shared_ptr<MenuBarStyle> style);
    virtual ~MenuBar() noexcept = default;

    void setWidth(float width);
    const sf::Vector2f& getSize() const;

    void addMenu(const MenuList& menu);    // FIXME add with an optional position instead of just the end? #########################
    bool removeMenu(const sf::String& name);
    const std::vector<MenuList>& getMenus() const;
    void removeAllMenus();

    void setStyle(std::shared_ptr<MenuBarStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    std::shared_ptr<MenuBarStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseHovering(const sf::Vector2f& mouseLocal) const override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;

protected:
    MenuBar(std::shared_ptr<MenuBarStyle> style);

private:
    void updateMenuBar();
    void updateMenu(MenuList& menu);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<MenuBarStyle> style_;
    bool styleCopied_;

    sf::Vector2f barSize_;
    std::vector<MenuList> menus_;
    size_t activeMenu_;
};

}
