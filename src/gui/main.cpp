#include "SFML/Graphics/Texture.hpp"
#include <gui/Button.h>
#include <gui/Gui.h>
#include <gui/Panel.h>
#include <gui/Signal.h>
#include <gui/themes/DefaultTheme.h>

#include <iostream>
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <utility>

void func() {
    std::cout << "func called\n";
}
void func2(int n) {
    std::cout << "func2 called with " << n << "\n";
}
void mouseClick(gui::Widget* w, const sf::Vector2f& mouseLocal) {
    std::cout << "mouseClick() at " << mouseLocal.x << ", " << mouseLocal.y << "\n";
}
void mouseEnter(gui::Widget* w) {
    std::cout << "mouseEnter() for " << w << "\n";
    //w->test__setSelected(true);
}
void mouseLeave(gui::Widget* w) {
    std::cout << "mouseLeave() for " << w << "\n";
    //w->test__setSelected(false);
}

sf::Color getRandColor() {
    return {static_cast<uint8_t>(rand() % 256), static_cast<uint8_t>(rand() % 256), static_cast<uint8_t>(rand() % 256)};
}

/**
 * Create a button that can be painted with randomly colored pixels by clicking
 * in the button area.
 */
auto createPaintButton(std::shared_ptr<gui::Theme> theme, sf::Image* paintImage, sf::Texture* paintTexture) {
    auto paintButton = gui::Button::create(theme);
    auto style = paintButton->getStyle();
    style->setFillColor(sf::Color::White);
    paintTexture->loadFromImage(*paintImage);
    style->setTexture(paintTexture, true);
    paintButton->setSize(sf::Vector2f(paintImage->getSize()));
    paintButton->onClick.connect([=](gui::Widget* w, const sf::Vector2f& mouseLocal) {
        std::cout << "(" << std::min(static_cast<unsigned int>(mouseLocal.x), paintImage->getSize().x)
        << ", " << std::min(static_cast<unsigned int>(mouseLocal.y), paintImage->getSize().y) << ")\n";
        paintImage->setPixel(
            std::min(static_cast<unsigned int>(mouseLocal.x), paintImage->getSize().x - 1),
            std::min(static_cast<unsigned int>(mouseLocal.y), paintImage->getSize().y - 1),
            getRandColor()
        );
        paintTexture->loadFromImage(*paintImage);
    });
    return paintButton;
}

int main() {
    std::cout << "Starting gui demo\n";
    sf::RenderWindow window(sf::VideoMode(400, 300), "GUI Demo");

    sf::CircleShape shape(100.0f);
    shape.setFillColor(sf::Color::Green);

    gui::Gui myGui;

    // FIXME: The lifetime of the theme is bound to the gui (due to font and stuff), make it stored in the gui? maybe not...
    // perhaps instead make it not a pointer (and pass ref to widget ctors) to make it clear.
    auto theme = gui::DefaultTheme::create();

    auto button = gui::Button::create(theme);
    button->setLabel(sf::String("hello!"));
    button->setRotation(18.0f);
    button->setOrigin(button->getSize() * 0.5f);
    //button->setSize(sf::Vector2f(40, 30));
    auto ret = button->onClick.connect(&mouseClick);
    std::cout << "connect gave " << ret << "\n";
    //ret = button->onClick.connect(&func2);
    //std::cout << "connect gave " << ret << "\n";
    ret = button->onClick.connect([&]{ window.close(); });
    std::cout << "connect gave " << ret << "\n";
    button->onClick.disconnect(ret);
    ret = button->onClick.connect([&]{ button->setLabel("nice to meet you"); });
    std::cout << "connect gave " << ret << "\n";
    myGui.addChild(button);

    std::cout << "button local bounds: L" << button->getLocalBounds().left << " T" << button->getLocalBounds().top << " W" << button->getLocalBounds().width << " H" << button->getLocalBounds().height << "\n";

    auto panel = gui::Panel::create(theme);
    panel->setSize({200, 150});
    panel->setOrigin(5.0f, 15.0f);
    panel->setPosition(50.0f, 50.0f);
    panel->setRotation(30.0f);
    myGui.addChild(panel);

    auto button2 = gui::Button::create(theme);
    button2->setPosition(panel->getSize() * 0.5f);
    button2->setRotation(-44.0f);
    button2->setLabel(sf::String("click me"));
    button2->setSize({160, 50});
    button2->setOrigin(button2->getSize() * 0.5f);
    button2->onClick.connect([&]{ std::cout << "button2 clicked\n"; /*button2->setLabel(button2->getLabel() + "?");*/ });
    button2->onClick.connect(&mouseClick);
    panel->addChild(button2);

    button->onMouseEnter.connect(&mouseEnter);
    button->onMouseLeave.connect(&mouseLeave);
    panel->onMouseEnter.connect(&mouseEnter);
    panel->onMouseLeave.connect(&mouseLeave);
    button2->onMouseEnter.connect(&mouseEnter);
    button2->onMouseLeave.connect(&mouseLeave);

    sf::Image paintImage;
    paintImage.create(70, 50, sf::Color::White);
    sf::Texture paintTexture;
    auto paintButton = createPaintButton(theme, &paintImage, &paintTexture);
    panel->addChild(paintButton);

    // Value ctors
    gui::Callback<int> cb(&func);
    gui::Callback<int> cb2(&func2);
    // Copy ctor
    gui::Callback<int> cb3 = cb;
    gui::Callback<int> cb4 = cb2;
    // Move ctor
    gui::Callback<int> cb5 = gui::Callback<int>(&func);
    gui::Callback<int> cb6 = gui::Callback<int>(&func2);
    // Swap function
    swap(cb5, cb6);
    swap(cb, cb6);
    swap(cb2, cb5);
    // Copy assignment
    cb3 = cb5;
    cb4 = cb6;
    // Move assignment
    cb3 = gui::Callback<int>(&func);
    cb4 = gui::Callback<int>(&func2);
    // Test with unordered_map and dtor
    std::unordered_map<unsigned int, gui::Callback<int>> stuff;
    stuff.emplace(1, &func);
    stuff.emplace(2, &func2);
    stuff.emplace(3, gui::Callback<int>(&func));
    stuff.clear();
    cb3.value<gui::Callback<int>::Tag::noArgs>()();
    cb4.value<gui::Callback<int>::Tag::withArgs>()(123);
    // Throws runtime error as intended
    //cb3.value<gui::Callback<int>::Tag::withArgs>()(123);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            myGui.handleEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        window.draw(shape);
        window.draw(myGui);
        window.display();
    }

    return 0;
}
