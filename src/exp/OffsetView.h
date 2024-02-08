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
    void setCenterOffset(int x, int y);
    void setCenterOffset(const sf::Vector2i& centerOffset);
    const sf::Vector2f& getCenter() const;
    const sf::Vector2f& getSize() const;
    float getViewDivisor() const;
    const sf::View& getView() const;
    const sf::Vector2i& getCenterOffset() const;
    template<typename Compare>
    void clampToView(const OffsetView& other, Compare comp);

private:
    float viewDivisor_;
    sf::View view_;
    sf::Vector2i centerOffset_;
};

template<typename Compare>
void OffsetView::clampToView(const OffsetView& other, Compare comp) {
    if (comp(static_cast<float>(centerOffset_.x), static_cast<float>(other.getCenterOffset().x))) {
        centerOffset_.x = other.getCenterOffset().x;
        view_.setCenter(other.getCenter().x, view_.getCenter().y);
    } else if (centerOffset_.x == other.getCenterOffset().x && comp(view_.getCenter().x, other.getCenter().x)) {
        view_.setCenter(other.getCenter().x, view_.getCenter().y);
    }

    if (comp(static_cast<float>(centerOffset_.y), static_cast<float>(other.getCenterOffset().y))) {
        centerOffset_.y = other.getCenterOffset().y;
        view_.setCenter(view_.getCenter().x, other.getCenter().y);
    } else if (centerOffset_.y == other.getCenterOffset().y && comp(view_.getCenter().y, other.getCenter().y)) {
        view_.setCenter(view_.getCenter().x, other.getCenter().y);
    }
}
