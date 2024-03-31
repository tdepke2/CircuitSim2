#include <gui/Gui.h>
#include <gui/Widget.h>

#include <cassert>
#include <stdexcept>
#include <string>


#include <iostream>



namespace gui {

Gui::Gui(sf::RenderWindow& window) :
    window_(window),
    redrawPending_(true),
    focusedWidget_(nullptr) {

    setSize(window.getSize());
}

void Gui::setSize(const sf::Vector2u& size) {
    if (!renderTexture_.create(size.x, size.y)) {
        throw std::runtime_error("Unable to create GUI render texture (size " + std::to_string(size.x) + " by " + std::to_string(size.y) + ").");
    }
    renderSprite_.setTexture(renderTexture_.getTexture(), true);
    requestRedraw();
}

sf::Vector2u Gui::getSize() const {
    return renderTexture_.getSize();
}

void Gui::setSmooth(bool smooth) {
    renderTexture_.setSmooth(smooth);
}

bool Gui::isSmooth() const {
    return renderTexture_.isSmooth();
}

void Gui::addChild(const std::shared_ptr<Widget>& child) {
    assert(child->getGui() == nullptr);
    children_.push_back(child);
    // The Gui itself is not added as a parent because it's not a Widget.
    child->setParentAndGui(nullptr, this);
}

void Gui::processEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr && widget->isEnabled()) {
            widget->handleMousePress(event.mouseButton.button, mouseGlobal);
        } else if (event.mouseButton.button <= sf::Mouse::Button::Middle) {
            requestWidgetFocus(nullptr);
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr && widget->isEnabled()) {
            widget->handleMouseRelease(event.mouseButton.button, mouseGlobal);
        }
    } else if (event.type == sf::Event::MouseMoved) {
        std::swap(widgetsUnderMouse_, lastWidgetsUnderMouse_);
        widgetsUnderMouse_.clear();

        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr) {
            widget->addWidgetUnderMouse(mouseGlobal);
            if (widget->isEnabled()) {
                widget->handleMouseMove(mouseGlobal);
            }
        }
        for (const auto& w : lastWidgetsUnderMouse_) {
            if (widgetsUnderMouse_.count(w) == 0 && w->isEnabled() && w->isMouseHovering()) {
                w->handleMouseLeft();
            }
        }
    } else if (event.type == sf::Event::MouseWheelScrolled) {
        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseWheelScroll.x, event.mouseWheelScroll.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr) {
            if (widget->isEnabled()) {
                widget->handleMouseWheelScroll(event.mouseWheelScroll.wheel, event.mouseWheelScroll.delta, mouseGlobal);
            }
        }
    } else if (event.type == sf::Event::MouseLeft) {
        for (const auto& w : widgetsUnderMouse_) {
            if (w->isEnabled() && w->isMouseHovering()) {
                w->handleMouseLeft();
            }
        }
        widgetsUnderMouse_.clear();
        lastWidgetsUnderMouse_.clear();
    } else if (event.type == sf::Event::TextEntered) {
        std::cout << "char code " << event.text.unicode << "\n";
        if (focusedWidget_ && focusedWidget_->isEnabled()) {
            focusedWidget_->handleTextEntered(event.text.unicode);
        }
    } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            requestWidgetFocus(nullptr);
        } else if (focusedWidget_ && focusedWidget_->isEnabled()) {
            focusedWidget_->handleKeyPressed(event.key);
        }
    } else if (event.type == sf::Event::Resized) {
        onWindowResized.emit(this, window_, sf::Vector2u(event.size.width, event.size.height));
    }
}

void Gui::addWidgetUnderMouse(std::shared_ptr<Widget> widget) {
    widgetsUnderMouse_.emplace(widget);
    if (lastWidgetsUnderMouse_.count(widget) == 0 && widget->isEnabled() && !widget->isMouseHovering()) {
        widget->handleMouseEntered();
    }
}

void Gui::requestWidgetFocus(std::shared_ptr<Widget> widget) {
    if (focusedWidget_) {
        focusedWidget_->handleFocusChange(false);
        requestRedraw();
    }
    if (widget) {
        widget->handleFocusChange(true);
        requestRedraw();
    }
    focusedWidget_ = widget;
}

void Gui::requestRedraw() const {
    redrawPending_ = true;
}

void Gui::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (redrawPending_) {
        std::cout << "Gui redraw...\n";
        renderTexture_.clear({0, 0, 0, 0});
        for (const auto& child : getChildren()) {
            renderTexture_.draw(*child);
        }
        renderTexture_.display();
        redrawPending_ = false;
    }

    target.draw(renderSprite_, states);
}

}
