#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Panel.h>

namespace gui {

PanelStyle::PanelStyle(const Gui& gui) :
    Style(gui) {
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



std::shared_ptr<Panel> Panel::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<Panel>(new Panel(theme.getStyle<PanelStyle>("Panel"), name));
}
std::shared_ptr<Panel> Panel::create(std::shared_ptr<PanelStyle> style, const sf::String& name) {
    return std::shared_ptr<Panel>(new Panel(style, name));
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
bool Panel::isMouseIntersecting(const sf::Vector2f& mouseParent) const {
    return Widget::isMouseIntersecting(mouseParent);
}

Panel::Panel(std::shared_ptr<PanelStyle> style, const sf::String& name) :
    Group(name),
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
