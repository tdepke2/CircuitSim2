#include <gui/Gui.h>
#include <gui/Panel.h>
#include <gui/Theme.h>

namespace gui {

PanelStyle::PanelStyle(const Gui& gui) :
    gui_(gui) {
}

// sf::Shape interface.
void PanelStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void PanelStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
    gui_.requestRedraw();
}
void PanelStyle::setFillColor(const sf::Color& color) {
    rect_.setFillColor(color);
    gui_.requestRedraw();
}
void PanelStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
    gui_.requestRedraw();
}
void PanelStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* PanelStyle::getTexture() const {
    return rect_.getTexture();
}
const sf::IntRect& PanelStyle::getTextureRect() const {
    return rect_.getTextureRect();
}
const sf::Color& PanelStyle::getFillColor() const {
    return rect_.getFillColor();
}
const sf::Color& PanelStyle::getOutlineColor() const {
    return rect_.getOutlineColor();
}
float PanelStyle::getOutlineThickness() const {
    return rect_.getOutlineThickness();
}

std::shared_ptr<PanelStyle> PanelStyle::clone() const {
    return std::make_shared<PanelStyle>(*this);
}



std::shared_ptr<Panel> Panel::create(const Theme& theme) {
    return std::shared_ptr<Panel>(new Panel(theme.getPanelStyle()));
}
std::shared_ptr<Panel> Panel::create(std::shared_ptr<PanelStyle> style) {
    return std::shared_ptr<Panel>(new Panel(style));
}

void Panel::setSize(const sf::Vector2f& size) {
    size_ = size;
    requestRedraw();
}
const sf::Vector2f& Panel::getSize() const {
    return size_;
}

void Panel::setStyle(std::shared_ptr<PanelStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<PanelStyle> Panel::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect Panel::getLocalBounds() const {
    return {-getOrigin(), size_};
}
void Panel::handleMouseMove(const sf::Vector2f& mouseParent) {
    Container::handleMouseMove(mouseParent);
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget != nullptr) {
        widget->handleMouseMove(mouseLocal);
    }
}
void Panel::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget != nullptr) {
        widget->handleMousePress(button, mouseLocal);
    } else {
        Container::handleMousePress(button, mouseParent);
    }
}
void Panel::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget != nullptr) {
        widget->handleMouseRelease(button, mouseLocal);
    } else {
        Container::handleMouseRelease(button, mouseParent);
    }
}

Panel::Panel(std::shared_ptr<PanelStyle> style) :
    style_(style),
    styleCopied_(false) {
}

void Panel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->rect_.setSize(size_);
    target.draw(style_->rect_, states);

    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
