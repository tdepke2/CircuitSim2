#pragma once

#include <SFML/Graphics.hpp>

/**
 * Combines an `sf::View` with an integer offset. This allows the center of the
 * view to have a very large magnitude without paying for loss of precision from
 * float types.
 * 
 * The center of the view member will be kept in a range from half of the view
 * size to half the view size plus the view divisor. Calculating the actual
 * center (with rounding errors) can be done with the following:
 * `getCenter() + sf::Vector2f(getCenterOffset()) * getViewDivisor()`
 */
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
