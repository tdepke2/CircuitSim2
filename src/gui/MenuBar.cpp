#include <gui/Gui.h>
#include <gui/MenuBar.h>
#include <gui/Theme.h>








#include <iostream>




namespace gui {

MenuBarStyle::MenuBarStyle(const Gui& gui) :
    gui_(gui) {
}

// sf::Shape interface.
void MenuBarStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    menu_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void MenuBarStyle::setTextureRect(const sf::IntRect& rect) {
    menu_.setTextureRect(rect);
    gui_.requestRedraw();
}
void MenuBarStyle::setFillColor(const sf::Color& color) {
    menu_.setFillColor(color);
    gui_.requestRedraw();
}
void MenuBarStyle::setOutlineColor(const sf::Color& color) {
    menu_.setOutlineColor(color);
    gui_.requestRedraw();
}
void MenuBarStyle::setOutlineThickness(float thickness) {
    menu_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* MenuBarStyle::getTexture() const {
    return menu_.getTexture();
}
const sf::IntRect& MenuBarStyle::getTextureRect() const {
    return menu_.getTextureRect();
}
const sf::Color& MenuBarStyle::getFillColor() const {
    return menu_.getFillColor();
}
const sf::Color& MenuBarStyle::getOutlineColor() const {
    return menu_.getOutlineColor();
}
float MenuBarStyle::getOutlineThickness() const {
    return menu_.getOutlineThickness();
}

// sf::Text interface.
void MenuBarStyle::setFont(const sf::Font& font) {
    text_.setFont(font);
    gui_.requestRedraw();
}
void MenuBarStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
    gui_.requestRedraw();
}
void MenuBarStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
    gui_.requestRedraw();
}
void MenuBarStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
    gui_.requestRedraw();
}
void MenuBarStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
    gui_.requestRedraw();
}
void MenuBarStyle::setTextFillColor(const sf::Color& color) {
    text_.setFillColor(color);
    gui_.requestRedraw();
}
const sf::Font* MenuBarStyle::getFont() const {
    return text_.getFont();
}
unsigned int MenuBarStyle::getCharacterSize() const {
    return text_.getCharacterSize();
}
float MenuBarStyle::getLineSpacing() const {
    return text_.getLineSpacing();
}
float MenuBarStyle::getLetterSpacing() const {
    return text_.getLetterSpacing();
}
uint32_t MenuBarStyle::getTextStyle() const {
    return text_.getStyle();
}
const sf::Color& MenuBarStyle::getTextFillColor() const {
    return text_.getFillColor();
}

void MenuBarStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Vector3f& MenuBarStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<MenuBarStyle> MenuBarStyle::clone() const {
    return std::make_shared<MenuBarStyle>(*this);
}



std::shared_ptr<MenuBar> MenuBar::create(const Theme& theme) {
    return std::shared_ptr<MenuBar>(new MenuBar(theme.getMenuBarStyle()));
}
std::shared_ptr<MenuBar> MenuBar::create(std::shared_ptr<MenuBarStyle> style) {
    return std::shared_ptr<MenuBar>(new MenuBar(style));
}

void MenuBar::setWidth(float width) {
    barSize_.x = width;
    barSize_.y = 2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize();
    requestRedraw();
}
const sf::Vector2f& MenuBar::getSize() const {
    return barSize_;
}

void MenuBar::addMenu(const MenuList& menu) {
    menus_.emplace_back(menu);
    updateMenuBar();
}

bool MenuBar::removeMenu(const sf::String& name) {
    for (auto it = menus_.cbegin(); it != menus_.cend(); ++it) {
        if (it->name == name) {
            menus_.erase(it);
            updateMenuBar();
            return true;
        }
    }
    return false;
}

const std::vector<MenuList>& MenuBar::getMenus() const {
    return menus_;
}

void MenuBar::removeAllMenus() {
    menus_.clear();
}

void MenuBar::setStyle(std::shared_ptr<MenuBarStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<MenuBarStyle> MenuBar::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect MenuBar::getLocalBounds() const {
    return {-getOrigin(), barSize_};
}
bool MenuBar::isMouseHovering(const sf::Vector2f& mouseLocal) const {
    return getLocalBounds().contains(toLocalSpace(mouseLocal));
    //???


}
void MenuBar::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {
    //???
}
void MenuBar::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {
    //???
}

MenuBar::MenuBar(std::shared_ptr<MenuBarStyle> style) :
    style_(style),
    styleCopied_(false) {
}

void MenuBar::updateMenuBar() {
    sf::Vector2f lastLabelPosition(style_->textPadding_.x, style_->textPadding_.y);
    for (auto& menu : menus_) {
        menu.labelPosition_ = lastLabelPosition;
        style_->text_.setString(menu.name);
        const auto bounds = style_->text_.getLocalBounds();
        lastLabelPosition.x += 2.0f * (bounds.left + style_->textPadding_.x) + bounds.width;
    }
    requestRedraw();
}

void MenuBar::updateMenu(MenuList& menu) {

}

void MenuBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->menu_.setSize(barSize_);
    target.draw(style_->menu_, states);
    for (const auto& menu : menus_) {
        style_->text_.setString(menu.name);
        style_->text_.setPosition(menu.labelPosition_);
        target.draw(style_->text_, states);
    }
}

}
