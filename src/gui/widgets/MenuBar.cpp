#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/MenuBar.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {

sf::Vector2f roundVector2f(sf::Vector2f v) {
    return {std::round(v.x), std::round(v.y)};
}

}

namespace gui {

MenuBarStyle::MenuBarStyle(const Gui& gui) :
    Style(gui) {
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
    textColor_ = color;
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
    return textColor_;
}

void MenuBarStyle::setBarTextPadding(const sf::Vector3f& padding) {
    barTextPadding_ = padding;
    gui_.requestRedraw();
}
void MenuBarStyle::setMenuTextPadding(const sf::Vector3f& padding) {
    menuTextPadding_ = padding;
    gui_.requestRedraw();
}
void MenuBarStyle::setMinLeftRightTextWidth(float width) {
    minLeftRightTextWidth_ = width;
    gui_.requestRedraw();
}
void MenuBarStyle::setDisabledTextFillColor(const sf::Color& color) {
    disabledTextColor_ = color;
    gui_.requestRedraw();
}
void MenuBarStyle::setHighlightFillColor(const sf::Color& color) {
    highlightColor_ = color;
    gui_.requestRedraw();
}
void MenuBarStyle::setDisabledHighlightFillColor(const sf::Color& color) {
    disabledHighlightColor_ = color;
    gui_.requestRedraw();
}
const sf::Vector3f& MenuBarStyle::getBarTextPadding() const {
    return barTextPadding_;
}
const sf::Vector3f& MenuBarStyle::getMenuTextPadding() const {
    return menuTextPadding_;
}
float MenuBarStyle::getMinLeftRightTextWidth() const {
    return minLeftRightTextWidth_;
}
const sf::Color& MenuBarStyle::getDisabledTextFillColor() const {
    return disabledTextColor_;
}
const sf::Color& MenuBarStyle::getHighlightFillColor() const {
    return highlightColor_;
}
const sf::Color& MenuBarStyle::getDisabledHighlightFillColor() const {
    return disabledHighlightColor_;
}

std::shared_ptr<MenuBarStyle> MenuBarStyle::clone() const {
    return std::make_shared<MenuBarStyle>(*this);
}



std::shared_ptr<MenuBar> MenuBar::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<MenuBar>(new MenuBar(theme.getStyle<MenuBarStyle>("MenuBar"), name));
}
std::shared_ptr<MenuBar> MenuBar::create(std::shared_ptr<MenuBarStyle> style, const sf::String& name) {
    return std::shared_ptr<MenuBar>(new MenuBar(style, name));
}

void MenuBar::setWidth(float width) {
    barSize_.x = width;
    barSize_.y = 2.0f * style_->barTextPadding_.y + style_->barTextPadding_.z * style_->getCharacterSize();
    requestRedraw();
}
const sf::Vector2f& MenuBar::getSize() const {
    return barSize_;
}

void MenuBar::insertMenu(const MenuList& menu) {
    insertMenu(menu, menus_.size());
}

void MenuBar::insertMenu(const MenuList& menu, size_t index) {
    menus_.emplace(menus_.begin() + std::min(index, menus_.size()), menu);
    updateMenuBar();
    selectMenu(selectedMenu_, false);
}

void MenuBar::removeMenu(size_t index) {
    menus_.erase(menus_.begin() + std::min(index, menus_.size() - 1));
    updateMenuBar();
    selectMenu(selectedMenu_ < static_cast<int>(menus_.size()) ? selectedMenu_ : -1, false);
}

void MenuBar::setMenu(const MenuList& menu, size_t index) {
    menus_[std::min(index, menus_.size())] = menu;
    updateMenuBar();
    selectMenu(selectedMenu_, false);
}

const std::vector<MenuList>& MenuBar::getMenus() const {
    return menus_;
}

void MenuBar::removeAllMenus() {
    menus_.clear();
    updateMenuBar();
    selectMenu(-1, false);
}

const MenuList& MenuBar::findMenu(const sf::String& name) const {
    return menus_[findMenuIndex(name)];
}

size_t MenuBar::findMenuIndex(const sf::String& name) const {
    for (size_t i = 0; i < menus_.size(); ++i) {
        if (menus_[i].name == name) {
            return i;
        }
    }
    throw std::out_of_range("MenuBar::findMenuIndex");
}

bool MenuBar::isMenuOpen() const {
    return menuIsOpen_;
}

void MenuBar::closeAllMenus() {
    selectMenu(-1, false);
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
bool MenuBar::isMouseIntersecting(const sf::Vector2f& mouseParent) const {
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (menuIsOpen_) {
        sf::FloatRect menuBounds(-getOrigin() + getOpenMenuPosition(), menus_[selectedMenu_].menuSize_);
        return getLocalBounds().contains(mouseLocal) || menuBounds.contains(mouseLocal);
    } else {
        return getLocalBounds().contains(mouseLocal);
    }
}

bool MenuBar::handleMouseMove(const sf::Vector2f& mouseParent) {
    if (baseClass::handleMouseMove(mouseParent)) {
        return true;
    } else if (!isMouseIntersecting(mouseParent)) {
        return false;
    }
    const auto mouseLocal = toLocalOriginSpace(mouseParent);

    mouseUpdate(false, false, mouseLocal);
    return isFocused();
}
void MenuBar::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    baseClass::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Middle) {
        mouseUpdate(true, true, mouseLocal);
        onClick.emit(this, mouseLocal);
    }
    onMousePress.emit(this, button, mouseLocal);
}
void MenuBar::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Middle) {
        mouseUpdate(true, false, mouseLocal);
    }
    onMouseRelease.emit(this, button, mouseLocal);

    baseClass::handleMouseRelease(button, mouseParent);
}

void MenuBar::handleMouseLeft() {
    if (!menuIsOpen_) {
        selectMenu(-1, false);
    } else {
        selectMenuItem(-1);
    }
    baseClass::handleMouseLeft();
}

void MenuBar::handleFocusChange(bool focused) {
    baseClass::handleFocusChange(focused);
    if (!focused) {
        selectMenu(-1, false);
    }
}

MenuBar::MenuBar(std::shared_ptr<MenuBarStyle> style, const sf::String& name) :
    baseClass(name),
    style_(style),
    styleCopied_(false),
    selectedMenu_(-1),
    menuIsOpen_(false) {
}

void MenuBar::updateMenuBar() {
    sf::Vector2f lastLabelPosition(style_->barTextPadding_.x, style_->barTextPadding_.y);
    for (auto& menu : menus_) {
        menu.labelPosition_ = roundVector2f(lastLabelPosition);
        style_->text_.setString(menu.name);
        const auto bounds = style_->text_.getLocalBounds();
        menu.labelWidth_ = bounds.left + bounds.width;
        lastLabelPosition.x += 2.0f * style_->barTextPadding_.x + menu.labelWidth_;
    }
    requestRedraw();
}

void MenuBar::updateMenu(int index) {
    auto& menu = menus_[index];
    float maxMenuWidth = 0.0f;
    sf::Vector2f lastLeftItemPosition = getOpenMenuPosition() + sf::Vector2f(style_->menuTextPadding_.x, style_->menuTextPadding_.y);

    for (auto& menuItem : menu.items) {
        style_->text_.setString(menuItem.leftText);
        const auto leftTextBounds = style_->text_.getLocalBounds();
        style_->text_.setString(menuItem.rightText);
        const auto rightTextBounds = style_->text_.getLocalBounds();
        maxMenuWidth = std::max(
            maxMenuWidth,
            2.0f * style_->menuTextPadding_.x + leftTextBounds.left + leftTextBounds.width + style_->minLeftRightTextWidth_ + rightTextBounds.left + rightTextBounds.width
        );

        menuItem.leftPosition_ = roundVector2f(lastLeftItemPosition);
        menuItem.rightPosition_ = roundVector2f({rightTextBounds.left + rightTextBounds.width, lastLeftItemPosition.y});
        lastLeftItemPosition.y += 2.0f * style_->menuTextPadding_.y + style_->menuTextPadding_.z * style_->getCharacterSize();
    }
    for (auto& menuItem : menu.items) {
        menuItem.rightPosition_.x = std::round(menuItem.leftPosition_.x + maxMenuWidth - menuItem.rightPosition_.x - 2.0f * style_->menuTextPadding_.x);
    }
    menu.menuSize_ = {maxMenuWidth, lastLeftItemPosition.y - getOpenMenuPosition().y - style_->menuTextPadding_.y};
    menu.selectedItem_ = -1;
    if (selectedMenu_ == index) {
        requestRedraw();
    }
}

void MenuBar::selectMenu(int index, bool isOpen) {
    if (selectedMenu_ != index || menuIsOpen_ != isOpen) {
        selectedMenu_ = index;
        menuIsOpen_ = isOpen;
        if (isOpen && index != -1) {
            updateMenu(index);
            sendToFront();
        }
        requestRedraw();
    }
}

void MenuBar::selectMenuItem(int index) {
    if (menuIsOpen_ && menus_[selectedMenu_].selectedItem_ != index) {
        menus_[selectedMenu_].selectedItem_ = index;
        requestRedraw();
    }
}

sf::Vector2f MenuBar::getOpenMenuPosition() const {
    return {menus_[selectedMenu_].labelPosition_.x - style_->barTextPadding_.x, barSize_.y};
}

void MenuBar::mouseUpdate(bool mouseClick, bool mouseDown, const sf::Vector2f& mouseLocal) {
    if (mouseLocal.y + getOrigin().y < barSize_.y) {
        // Mouse is over menu bar.
        selectMenuItem(-1);
        for (size_t i = 0; i < menus_.size(); ++i) {
            if (mouseLocal.x + getOrigin().x < menus_[i].labelPosition_.x + menus_[i].labelWidth_ + style_->barTextPadding_.x) {
                selectMenu(static_cast<int>(i), mouseDown != menuIsOpen_);
                return;
            }
        }
        if (mouseDown || !menuIsOpen_) {
            selectMenu(-1, false);
        }
    } else if (menuIsOpen_) {
        // Mouse is in a menu.
        const auto& menu = menus_[selectedMenu_];
        size_t selectedMenuItem = menu.items.size() - 1;
        for (size_t i = 1; i < menu.items.size(); ++i) {
            if (mouseLocal.y + getOrigin().y < menu.items[i].leftPosition_.y - style_->menuTextPadding_.y) {
                selectedMenuItem = i - 1;
                break;
            }
        }
        selectMenuItem(static_cast<int>(selectedMenuItem));
        if (mouseClick && !mouseDown && menu.items[selectedMenuItem].enabled) {
            onMenuItemClick.emit(this, menu, selectedMenuItem);
            selectMenu(-1, false);
        }
    }
}

void MenuBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->bar_.setSize(barSize_);
    target.draw(style_->bar_, states);
    if (selectedMenu_ != -1) {
        style_->highlight_.setFillColor(style_->highlightColor_);
        style_->highlight_.setPosition(menus_[selectedMenu_].labelPosition_.x - style_->barTextPadding_.x, 0.0f);
        style_->highlight_.setSize({2.0f * style_->barTextPadding_.x + menus_[selectedMenu_].labelWidth_, barSize_.y});
        target.draw(style_->highlight_, states);
    }

    style_->text_.setFillColor(style_->textColor_);
    for (const auto& menu : menus_) {
        style_->text_.setString(menu.name);
        style_->text_.setPosition(menu.labelPosition_);
        target.draw(style_->text_, states);
    }

    if (menuIsOpen_) {
        const auto& menu = menus_[selectedMenu_];
        style_->menu_.setSize(menu.menuSize_);
        style_->menu_.setPosition(getOpenMenuPosition());
        target.draw(style_->menu_, states);

        if (menu.selectedItem_ != -1) {
            const auto& menuItem = menu.items[menu.selectedItem_];
            style_->highlight_.setFillColor(menuItem.enabled ? style_->highlightColor_ : style_->disabledHighlightColor_);
            style_->highlight_.setPosition(menuItem.leftPosition_.x - style_->menuTextPadding_.x, menuItem.leftPosition_.y - style_->menuTextPadding_.y);
            style_->highlight_.setSize({menu.menuSize_.x, 2.0f * style_->menuTextPadding_.y + style_->menuTextPadding_.z * style_->getCharacterSize()});
            target.draw(style_->highlight_, states);
        }

        for (const auto& menuItem : menu.items) {
            style_->text_.setFillColor(menuItem.enabled ? style_->textColor_ : style_->disabledTextColor_);
            style_->text_.setString(menuItem.leftText);
            style_->text_.setPosition(menuItem.leftPosition_);
            target.draw(style_->text_, states);
            style_->text_.setString(menuItem.rightText);
            style_->text_.setPosition(menuItem.rightPosition_);
            target.draw(style_->text_, states);
        }
    }
}

}
