#include <gui/Debug.h>
#include <gui/Gui.h>
#include <gui/Widget.h>

#include <cassert>
#include <stdexcept>
#include <stack>
#include <string>
#include <utility>

namespace gui {

namespace {

/**
 * Transform a mouse position into the parent space of the `focusedWidget` so we
 * can pass an event to this widget. Returns false if nothing focused or the
 * mouse is already over the focused widget.
 */
std::pair<sf::Vector2f, bool> propagateMouseEvent(const sf::Vector2f& mouseGlobal, const Widget* widgetUnderMouse, const Widget* focusedWidget) {
    if (focusedWidget != nullptr && focusedWidget->isEnabled()) {
        auto widget = focusedWidget->getParent();
        std::stack<Widget*> focusedBranch;
        while (widget != nullptr) {
            focusedBranch.push(widget);
            widget = widget->getParent();
        }
        auto mouseLocal = mouseGlobal;
        while (!focusedBranch.empty()) {
            mouseLocal = focusedBranch.top()->toLocalSpace(mouseLocal);
            auto container = dynamic_cast<ContainerBase*>(focusedBranch.top());
            if (widgetUnderMouse != nullptr && container != nullptr) {
                widgetUnderMouse = container->getWidgetUnderMouse(mouseLocal);
            }
            focusedBranch.pop();
        }

        if (focusedWidget != widgetUnderMouse) {
            //GUI_DEBUG << "The focused widget is not under mouse, send event. mouseLocal = " << mouseLocal.x << ", " << mouseLocal.y << "\n";
            return {mouseLocal, true};
        }
        // else {
        //    GUI_DEBUG << "Focused widget under mouse already.\n";
        //}
    }
    return {{0.0f, 0.0f}, false};
}

}

Gui::Gui(sf::RenderWindow& window, unsigned int antialiasingLevel) :
    window_(window),
    antialiasingLevel_(antialiasingLevel),
    redrawPending_(true),
    focusedWidget_(nullptr) {

    setSize(window.getSize());
}

void Gui::setSize(const sf::Vector2u& size) {
    if (!renderTexture_.create(size.x, size.y, sf::ContextSettings(0, 0, antialiasingLevel_))) {
        throw std::runtime_error(
            "Unable to create GUI render texture (size " + std::to_string(size.x) + " by " + std::to_string(size.y) +
            " with anti-aliasing level " + std::to_string(antialiasingLevel_) + ")."
        );
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

void Gui::addChild(std::shared_ptr<Widget> child) {
    assert(child->getGui() == nullptr);
    children_.push_back(child);
    // The Gui itself is not added as a parent because it's not a Widget.
    child->setParentAndGui(nullptr, this);
}

bool Gui::processEvent(const sf::Event& event) {
    bool eventConsumed = false;
    if (event.type == sf::Event::MouseButtonPressed) {
        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr && widget->isEnabled()) {
            widget->handleMousePress(event.mouseButton.button, mouseGlobal);
            eventConsumed = true;
        } else if (event.mouseButton.button <= sf::Mouse::Middle) {
            requestWidgetFocus(nullptr);
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr && widget->isEnabled()) {
            widget->handleMouseRelease(event.mouseButton.button, mouseGlobal);
            eventConsumed = true;
        }
    } else if (event.type == sf::Event::MouseMoved) {
        std::swap(widgetsUnderMouse_, lastWidgetsUnderMouse_);
        widgetsUnderMouse_.clear();

        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr) {
            widget->addWidgetUnderMouse(mouseGlobal);
            if (widget->isEnabled()) {
                eventConsumed = widget->handleMouseMove(mouseGlobal);
            }
        }
        if (!eventConsumed) {
            auto mouseLocal = propagateMouseEvent(mouseGlobal, widget, focusedWidget_.get());
            if (mouseLocal.second) {
                eventConsumed = focusedWidget_->handleMouseMove(mouseLocal.first);
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
                eventConsumed = widget->handleMouseWheelScroll(event.mouseWheelScroll.wheel, event.mouseWheelScroll.delta, mouseGlobal);
            }
        }
        if (!eventConsumed) {
            auto mouseLocal = propagateMouseEvent(mouseGlobal, widget, focusedWidget_.get());
            if (mouseLocal.second) {
                eventConsumed = focusedWidget_->handleMouseWheelScroll(event.mouseWheelScroll.wheel, event.mouseWheelScroll.delta, mouseLocal.first);
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
        GUI_DEBUG << "char code " << event.text.unicode << "\n";
        // Key events are passed to the child first (focusedWidget_) and then
        // passed along the chain of parents until it's consumed. This is
        // similar to how it works for mouse events but the order is reversed.
        // As a side effect, disabled widgets do not block propagation of key
        // events.
        if (focusedWidget_) {
            auto widget = focusedWidget_.get();
            do {
                if (widget->isEnabled()) {
                    eventConsumed = widget->handleTextEntered(event.text.unicode);
                }
                widget = widget->getParent();
            } while (!eventConsumed && widget != nullptr);
        }
    } else if (event.type == sf::Event::KeyPressed) {
        if (focusedWidget_) {
            auto widget = focusedWidget_.get();
            do {
                if (widget->isEnabled()) {
                    eventConsumed = widget->handleKeyPressed(event.key);
                }
                widget = widget->getParent();
            } while (!eventConsumed && widget != nullptr);

            if (!eventConsumed && event.key.code == sf::Keyboard::Escape) {
                requestWidgetFocus(nullptr);
                eventConsumed = true;
            }
        }
    } else if (event.type == sf::Event::Resized) {
        onWindowResized.emit(this, window_, sf::Vector2u(event.size.width, event.size.height));
    }
    return eventConsumed;
}

void Gui::addWidgetUnderMouse(std::shared_ptr<Widget> widget) {
    widgetsUnderMouse_.emplace(widget);
    if (lastWidgetsUnderMouse_.count(widget) == 0 && widget->isEnabled() && !widget->isMouseHovering()) {
        widget->handleMouseEntered();
    }
}

void Gui::requestWidgetFocus(std::shared_ptr<Widget> widget) {
    if (widget && !widget->isFocusable()) {
        return;
    }
    if (focusedWidget_) {
        focusedWidget_->handleFocusChange(false);
    }
    if (widget) {
        widget->handleFocusChange(true);
    }
    focusedWidget_ = widget;
}

void Gui::requestRedraw() const {
    redrawPending_ = true;
}

void Gui::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (redrawPending_) {
        GUI_DEBUG << "Gui redraw...\n";
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
