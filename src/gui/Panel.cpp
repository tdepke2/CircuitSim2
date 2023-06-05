#include <gui/Gui.h>
#include <gui/Panel.h>
#include <gui/Theme.h>

namespace gui {


// sf::Shape interface.
void PanelStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
}
void PanelStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
}
void PanelStyle::setFillColor(const sf::Color& color) {
    rect_.setFillColor(color);
}
void PanelStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
}
void PanelStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
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



std::shared_ptr<Panel> Panel::create(std::shared_ptr<Theme> theme) {
    return std::shared_ptr<Panel>(new Panel(theme->getPanelStyle()));
}
std::shared_ptr<Panel> Panel::create(std::shared_ptr<PanelStyle> style) {
    return std::shared_ptr<Panel>(new Panel(style));
}

void Panel::setSize(const sf::Vector2f& size) {
    size_ = size;
}
const sf::Vector2f& Panel::getSize() const {
    return size_;
}

void Panel::setStyle(std::shared_ptr<PanelStyle> style) {
    style_ = style;
    styleCopied_ = false;
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
void Panel::handleMouseMove(const sf::Vector2f& mouseLocal) {
    auto newMouseLocal = getInverseTransform().transformPoint(mouseLocal);
    auto widget = getWidgetUnderMouse(newMouseLocal);
    if (widget != nullptr) {
        widget->handleMouseMove(newMouseLocal);
    }
    Widget::handleMouseMove(mouseLocal);
}
void Panel::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {
    if (!isEnabled()) {
        return;
    }
    auto newMouseLocal = getInverseTransform().transformPoint(mouseLocal);
    auto widget = getWidgetUnderMouse(newMouseLocal);
    if (widget != nullptr) {
        widget->handleMousePress(button, newMouseLocal);
    } else {
        Widget::handleMousePress(button, mouseLocal);
    }
}
void Panel::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {
    if (!isEnabled()) {
        return;
    }
    auto newMouseLocal = getInverseTransform().transformPoint(mouseLocal);
    auto widget = getWidgetUnderMouse(newMouseLocal);
    if (widget != nullptr) {
        widget->handleMouseRelease(button, newMouseLocal);
    } else {
        Widget::handleMouseRelease(button, mouseLocal);
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
