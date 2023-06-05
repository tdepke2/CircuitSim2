#include "gui/Container.h"
#include <gui/Gui.h>
#include <gui/Widget.h>



#include <iostream>



namespace gui {

Gui::Gui(sf::RenderWindow& window) :
    window_(window),
    focusedWidget_(nullptr) {
}

void Gui::addChild(std::shared_ptr<Widget> child) {
    children_.push_back(child);
    child->setParent(nullptr);
    child->setGui(this);
}

void Gui::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr) {
            widget->handleMousePress(event.mouseButton.button, mouseGlobal);
        } else if (event.mouseButton.button <= sf::Mouse::Button::Middle) {
            requestWidgetFocus(nullptr);
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr) {
            widget->handleMouseRelease(event.mouseButton.button, mouseGlobal);
        }
    } else if (event.type == sf::Event::MouseMoved) {
        std::swap(widgetsUnderMouse_, lastWidgetsUnderMouse_);
        widgetsUnderMouse_.clear();

        const auto mouseGlobal = window_.mapPixelToCoords({event.mouseMove.x, event.mouseMove.y});
        auto widget = getWidgetUnderMouse(mouseGlobal);
        if (widget != nullptr) {
            widget->handleMouseMove(mouseGlobal);
        }
        for (const auto& w : lastWidgetsUnderMouse_) {
            if (widgetsUnderMouse_.count(w) == 0 && w->isEnabled()) {
                w->handleMouseLeft();
            }
        }
    } else if (event.type == sf::Event::MouseLeft) {
        for (const auto& w : widgetsUnderMouse_) {
            if (w->isEnabled()) {
                w->handleMouseLeft();
            }
        }
        widgetsUnderMouse_.clear();
        lastWidgetsUnderMouse_.clear();
    } else if (event.type == sf::Event::MouseWheelScrolled) {

    } else if (event.type == sf::Event::TextEntered) {
        std::cout << "char code " << event.text.unicode << "\n";
        if (focusedWidget_) {
            focusedWidget_->handleTextEntered(event.text.unicode);
        }
    } else if (event.type == sf::Event::KeyPressed) {
        
    }
}

void Gui::addWidgetUnderMouse(std::shared_ptr<Widget> widget) {
    widgetsUnderMouse_.emplace(widget);
    if (lastWidgetsUnderMouse_.count(widget) == 0 && widget->isEnabled()) {
        widget->handleMouseEntered();
    }
}

void Gui::requestWidgetFocus(std::shared_ptr<Widget> widget) {
    if (focusedWidget_) {
        focusedWidget_->handleFocusChange(false);
    }
    if (widget) {
        widget->handleFocusChange(true);
    }
    focusedWidget_ = widget;
}

void Gui::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
