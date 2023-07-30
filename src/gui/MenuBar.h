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

    friend class MenuBar;
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
    float labelWidth_;
    int selectedItem_;

    friend class MenuBar;
};

class MenuBarStyle {
public:
    MenuBarStyle(const Gui& gui);

    // sf::Shape interface.
    void setBarTexture(const sf::Texture* texture, bool resetRect = false);
    void setBarTextureRect(const sf::IntRect& rect);
    void setBarFillColor(const sf::Color& color);
    void setBarOutlineColor(const sf::Color& color);
    void setBarOutlineThickness(float thickness);
    const sf::Texture* getBarTexture() const;
    const sf::IntRect& getBarTextureRect() const;
    const sf::Color& getBarFillColor() const;
    const sf::Color& getBarOutlineColor() const;
    float getBarOutlineThickness() const;

    // sf::Shape interface.
    void setMenuTexture(const sf::Texture* texture, bool resetRect = false);
    void setMenuTextureRect(const sf::IntRect& rect);
    void setMenuFillColor(const sf::Color& color);
    void setMenuOutlineColor(const sf::Color& color);
    void setMenuOutlineThickness(float thickness);
    const sf::Texture* getMenuTexture() const;
    const sf::IntRect& getMenuTextureRect() const;
    const sf::Color& getMenuFillColor() const;
    const sf::Color& getMenuOutlineColor() const;
    float getMenuOutlineThickness() const;

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

    void setBarTextPadding(const sf::Vector3f& padding);
    void setMenuTextPadding(const sf::Vector3f& padding);
    void setMinLeftRightTextWidth(float width);
    void setHighlightFillColor(const sf::Color& color);
    const sf::Vector3f& getBarTextPadding() const;
    const sf::Vector3f& getMenuTextPadding() const;
    float getMinLeftRightTextWidth() const;
    const sf::Color& getHighlightFillColor() const;

    std::shared_ptr<MenuBarStyle> clone() const;

private:
    const Gui& gui_;
    sf::RectangleShape bar_, menu_, highlight_;
    sf::Text text_;
    sf::Vector3f barTextPadding_, menuTextPadding_;
    float minLeftRightTextWidth_;

    friend class MenuBar;
};

class MenuBar : public Widget {
public:
    static std::shared_ptr<MenuBar> create(const Theme& theme);
    static std::shared_ptr<MenuBar> create(std::shared_ptr<MenuBarStyle> style);
    virtual ~MenuBar() noexcept = default;

    void setWidth(float width);
    const sf::Vector2f& getSize() const;

    void insertMenu(const MenuList& menu);
    void insertMenu(const MenuList& menu, size_t index);
    void removeMenu(size_t index);
    void setMenu(const MenuList& menu, size_t index);
    const std::vector<MenuList>& getMenus() const;
    void removeAllMenus();

    const MenuList& findMenu(const sf::String& name) const;
    size_t findMenuIndex(const sf::String& name) const;

    void setStyle(std::shared_ptr<MenuBarStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    std::shared_ptr<MenuBarStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseHovering(const sf::Vector2f& mouseParent) const override;

    virtual void handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

    virtual void handleMouseLeft() override;
    virtual void handleFocusChange(bool focused) override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;
    Signal<Widget*, const MenuList&, size_t> onMenuItemClick;

protected:
    MenuBar(std::shared_ptr<MenuBarStyle> style);

private:
    void updateMenuBar();
    void updateMenu(int index);
    void selectMenu(int index, bool isOpen);
    void selectMenuItem(int index);
    sf::Vector2f getOpenMenuPosition() const;
    void mouseUpdate(bool clicked, const sf::Vector2f& mouseLocal);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<MenuBarStyle> style_;
    bool styleCopied_;

    sf::Vector2f barSize_;
    std::vector<MenuList> menus_;
    int selectedMenu_;
    bool menuIsOpen_;
};

}
