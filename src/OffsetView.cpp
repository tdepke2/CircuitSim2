#include <OffsetView.h>

#include <cmath>
#include <spdlog/spdlog.h>

OffsetView::OffsetView(float viewDivisor) :
    viewDivisor_(viewDivisor),
    view_(),
    centerOffset_() {
}

OffsetView::OffsetView(float viewDivisor, const sf::View& view) :
    viewDivisor_(viewDivisor),
    view_(),
    centerOffset_() {

    view_.setSize(view.getSize());
    setCenter(view.getCenter());
}

void OffsetView::setCenter(float x, float y) {
    sf::Vector2f halfView = view_.getSize() / 2.0f;
    sf::Vector2f modCenter(
        std::fmod(x - halfView.x, viewDivisor_),
        std::fmod(y - halfView.y, viewDivisor_)
    );
    modCenter.x += (modCenter.x < 0.0f ? viewDivisor_ : 0.0f) + halfView.x;
    modCenter.y += (modCenter.y < 0.0f ? viewDivisor_ : 0.0f) + halfView.y;

    centerOffset_.x += std::lround((x - modCenter.x) / viewDivisor_);
    centerOffset_.y += std::lround((y - modCenter.y) / viewDivisor_);

    view_.setCenter(modCenter);

    /*spdlog::debug("OffsetView center=({}, {})\toffset=({}, {})\tsize=({}, {})",
        getCenter().x, getCenter().y, getCenterOffset().x, getCenterOffset().y, getSize().x, getSize().y
    );*/
}

void OffsetView::setCenter(const sf::Vector2f& center) {
    setCenter(center.x, center.y);
}

void OffsetView::setSize(float width, float height) {
    view_.setSize(width, height);
    setCenter(view_.getCenter());
}

void OffsetView::setSize(const sf::Vector2f& size) {
    setSize(size.x, size.y);
}

void OffsetView::setCenterOffset(int x, int y) {
    centerOffset_.x = x;
    centerOffset_.y = y;
}

void OffsetView::setCenterOffset(const sf::Vector2i& centerOffset) {
    setCenterOffset(centerOffset.x, centerOffset.y);
}

const sf::Vector2f& OffsetView::getCenter() const {
    return view_.getCenter();
}

const sf::Vector2f& OffsetView::getSize() const {
    return view_.getSize();
}

float OffsetView::getViewDivisor() const {
    return viewDivisor_;
}

const sf::View& OffsetView::getView() const {
    return view_;
}

const sf::Vector2i& OffsetView::getCenterOffset() const {
    return centerOffset_;
}
