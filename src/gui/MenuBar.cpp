#include <gui/Gui.h>
#include <gui/MenuBar.h>
#include <gui/Theme.h>








#include <iostream>




namespace gui {

MenuBarStyle::MenuBarStyle(const Gui& gui) :
    gui_(gui) {
}

// sf::Shape interface.
void MenuBarStyle::setBarTexture(const sf::Texture* texture, bool resetRect) {
    bar_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void MenuBarStyle::setBarTextureRect(const sf::IntRect& rect) {
    bar_.setTextureRect(rect);
    gui_.requestRedraw();
}
void MenuBarStyle::setBarFillColor(const sf::Color& color) {
    bar_.setFillColor(color);
    gui_.requestRedraw();
}
void MenuBarStyle::setBarOutlineColor(const sf::Color& color) {
    bar_.setOutlineColor(color);
    gui_.requestRedraw();
}
void MenuBarStyle::setBarOutlineThickness(float thickness) {
    bar_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* MenuBarStyle::getBarTexture() const {
    return bar_.getTexture();
}
const sf::IntRect& MenuBarStyle::getBarTextureRect() const {
    return bar_.getTextureRect();
}
const sf::Color& MenuBarStyle::getBarFillColor() const {
    return bar_.getFillColor();
}
const sf::Color& MenuBarStyle::getBarOutlineColor() const {
    return bar_.getOutlineColor();
}
float MenuBarStyle::getBarOutlineThickness() const {
    return bar_.getOutlineThickness();
}

// sf::Shape interface.
void MenuBarStyle::setMenuTexture(const sf::Texture* texture, bool resetRect) {
    menu_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void MenuBarStyle::setMenuTextureRect(const sf::IntRect& rect) {
    menu_.setTextureRect(rect);
    gui_.requestRedraw();
}
void MenuBarStyle::setMenuFillColor(const sf::Color& color) {
    menu_.setFillColor(color);
    gui_.requestRedraw();
}
void MenuBarStyle::setMenuOutlineColor(const sf::Color& color) {
    menu_.setOutlineColor(color);
    gui_.requestRedraw();
}
void MenuBarStyle::setMenuOutlineThickness(float thickness) {
    menu_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* MenuBarStyle::getMenuTexture() const {
    return menu_.getTexture();
}
const sf::IntRect& MenuBarStyle::getMenuTextureRect() const {
    return menu_.getTextureRect();
}
const sf::Color& MenuBarStyle::getMenuFillColor() const {
    return menu_.getFillColor();
}
const sf::Color& MenuBarStyle::getMenuOutlineColor() const {
    return menu_.getOutlineColor();
}
float MenuBarStyle::getMenuOutlineThickness() const {
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

void MenuBarStyle::setBarTextPadding(const sf::Vector3f& padding) {
    barTextPadding_ = padding;
    gui_.requestRedraw();
}
void MenuBarStyle::setMenuTextPadding(const sf::Vector3f& padding) {
    menuTextPadding_ = padding;
    gui_.requestRedraw();
}
void MenuBarStyle::setHighlightFillColor(const sf::Color& color) {
    highlight_.setFillColor(color);
    gui_.requestRedraw();
}
const sf::Vector3f& MenuBarStyle::getBarTextPadding() const {
    return barTextPadding_;
}
const sf::Vector3f& MenuBarStyle::getMenuTextPadding() const {
    return menuTextPadding_;
}
const sf::Color& MenuBarStyle::getHighlightFillColor() const {
    return highlight_.getFillColor();
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
    barSize_.y = 2.0f * style_->barTextPadding_.y + style_->barTextPadding_.z * style_->getCharacterSize();
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
bool MenuBar::isMouseHovering(const sf::Vector2f& mouseParent) const {
    return getLocalBounds().contains(toLocalOriginSpace(mouseParent));
    //???


}

void MenuBar::handleMouseMove(const sf::Vector2f& mouseParent) {
    Widget::handleMouseMove(mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    std::cout << "mouseLocal = (" << mouseLocal.x << ", " << mouseLocal.y << ")\n";

    for (size_t i = 0; i < menus_.size(); ++i) {
        if (mouseLocal.x + getOrigin().x < menus_[i].labelPosition_.x + menus_[i].labelWidth_ + style_->barTextPadding_.x) {
            selectMenu(i);
            return;
        }
    }
    selectMenu(-1);
}
void MenuBar::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    //???
}
void MenuBar::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    //???
}

void MenuBar::handleMouseLeft() {
    selectMenu(-1);
    Widget::handleMouseLeft();
}

MenuBar::MenuBar(std::shared_ptr<MenuBarStyle> style) :
    style_(style),
    styleCopied_(false),
    selectedMenu_(-1),
    menuIsOpen_(false) {
}

void MenuBar::updateMenuBar() {
    sf::Vector2f lastLabelPosition(style_->barTextPadding_.x, style_->barTextPadding_.y);
    for (auto& menu : menus_) {
        menu.labelPosition_ = lastLabelPosition;
        style_->text_.setString(menu.name);
        const auto bounds = style_->text_.getLocalBounds();
        menu.labelWidth_ = bounds.left + bounds.width;
        lastLabelPosition.x += 2.0f * style_->barTextPadding_.x + menu.labelWidth_;
    }
    requestRedraw();
}

void MenuBar::updateMenu(MenuList& menu) {

}

void MenuBar::selectMenu(int index) {
    if (selectedMenu_ != index) {
        selectedMenu_ = index;
        requestRedraw();
    }
}

void MenuBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->menu_.setSize(barSize_);
    target.draw(style_->menu_, states);
    if (selectedMenu_ != -1) {
        style_->highlight_.setPosition(menus_[selectedMenu_].labelPosition_.x - style_->barTextPadding_.x, 0.0f);
        style_->highlight_.setSize({2.0f * style_->barTextPadding_.x + menus_[selectedMenu_].labelWidth_, barSize_.y});
        target.draw(style_->highlight_, states);
    }
    for (const auto& menu : menus_) {
        style_->text_.setString(menu.name);
        style_->text_.setPosition(menu.labelPosition_);
        target.draw(style_->text_, states);
    }
}

}
