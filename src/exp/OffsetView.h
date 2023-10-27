#pragma once

#include <SFML/Graphics.hpp>

class OffsetView {
public:
    OffsetView(float viewDivisor);
    OffsetView(float viewDivisor, const sf::View& view);
    void setCenter(float x, float y);
    void setCenter(const sf::Vector2f& center);
    void setSize(float width, float height);
    void setSize(const sf::Vector2f& size);
    const sf::Vector2f& getCenter() const;
    const sf::Vector2f& getSize() const;
    float getViewDivisor() const;
    const sf::View& getView() const;
    const sf::Vector2i& getCenterOffset() const;

private:
    float viewDivisor_;
    sf::View view_;
    sf::Vector2i centerOffset_;
};
