#include <gui/Gui.h>
#include <gui/Widget.h>



#include <iostream>



namespace gui {

Gui::Gui(sf::RenderWindow& window) :
    window_(window),
    redrawPending_(true),
    focusedWidget_(nullptr) {

    renderTexture_.create(window.getSize().x, window.getSize().y);
    renderSprite_.setTexture(renderTexture_.getTexture());
}

void Gui::setSmooth(bool smooth) {
    renderTexture_.setSmooth(smooth);
}

bool Gui::isSmooth() const {
    return renderTexture_.isSmooth();
}

void Gui::addChild(std::shared_ptr<Widget> child) {
    children_.push_back(child);
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
        if (focusedWidget_ && focusedWidget_->isEnabled()) {
            focusedWidget_->handleTextEntered(event.text.unicode);
        }
    } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            requestWidgetFocus(nullptr);
        } else if (focusedWidget_ && focusedWidget_->isEnabled()) {
            focusedWidget_->handleKeyPressed(event.key.code);
        }
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
            renderTexture_.draw(*child, states);
        }
        renderTexture_.display();
        redrawPending_ = false;
    }


    // FIXME is it necessary to pass the RenderStates to renderTexture_ above?



    target.draw(renderSprite_, states);
}

}
