#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/RadioButton.h>

#include <cmath>

namespace gui {

RadioButtonStyle::RadioButtonStyle(const Gui& gui) :
    CheckBoxStyle(gui) {
}

void RadioButtonStyle::setDiamond(bool diamond) {
    isDiamond_ = diamond;
    gui_.requestRedraw();
}
bool RadioButtonStyle::isDiamond() const {
    return isDiamond_;
}

std::shared_ptr<RadioButtonStyle> RadioButtonStyle::clone() const {
    return std::make_shared<RadioButtonStyle>(*this);
}



std::shared_ptr<RadioButton> RadioButton::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<RadioButton>(new RadioButton(theme.getStyle<RadioButtonStyle>("RadioButton"), name));
}
std::shared_ptr<RadioButton> RadioButton::create(std::shared_ptr<RadioButtonStyle> style, const sf::String& name) {
    return std::shared_ptr<RadioButton>(new RadioButton(style, name));
}

void RadioButton::setChecked(bool checked) {
    if (isChecked_ != checked) {
        isChecked_ = checked;
        requestRedraw();
    }

    if (!checked || getParent() == nullptr) {
        return;
    }
    for (const auto& child : getParent()->getChildren()) {
        auto childButton = dynamic_cast<RadioButton*>(child.get());
        if (childButton != nullptr && childButton->isChecked_ && childButton != this) {
            childButton->isChecked_ = false;
            requestRedraw();
        }
    }
}

bool RadioButton::isChecked() const {
    return isChecked_;
}

void RadioButton::uncheckRadioButtons() {
    if (getParent() == nullptr) {
        return;
    }
    for (const auto& child : getParent()->getChildren()) {
        auto childButton = dynamic_cast<RadioButton*>(child.get());
        if (childButton != nullptr && childButton->isChecked_) {
            childButton->isChecked_ = false;
            requestRedraw();
        }
    }
}

void RadioButton::setStyle(std::shared_ptr<RadioButtonStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<RadioButtonStyle> RadioButton::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

void RadioButton::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    if (button <= sf::Mouse::Middle && Button::isPressed()) {
        setChecked(true);
    }
    Button::handleMouseRelease(button, mouseParent);
}

RadioButton::RadioButton(std::shared_ptr<RadioButtonStyle> style, const sf::String& name) :
    Button(nullptr, name),
    style_(style),
    styleCopied_(false),
    isChecked_(false) {
}

void RadioButton::computeResize() const {
    style_->text_.setString(getLabel());
    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize()
    );
    size_.x += size_.y;
}

void RadioButton::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    if (getAutoResize()) {
        computeResize();
    } else {
        style_->text_.setString(getLabel());
    }
    if (style_->isDiamond_) {
        style_->rect_.setPosition(size_.y / 2.0f, 0.0f);
        style_->rect_.setRotation(45.0f);
        const float sideLength = size_.y / std::sqrt(2.0f);
        style_->rect_.setSize({sideLength, sideLength});
    } else {
        style_->rect_.setPosition(0.0f, 0.0f);
        style_->rect_.setRotation(0.0f);
        style_->rect_.setSize({size_.y, size_.y});
    }
    sf::Color fillColor = (isChecked_ ? style_->colorChecked_ : style_->colorUnchecked_);
    if (isMouseHovering()) {
        fillColor = Style::blendColors(style_->colorHover_, fillColor);
    }
    style_->rect_.setFillColor(fillColor);
    target.draw(style_->rect_, states);
    style_->text_.setPosition(style_->textPadding_.x + static_cast<int>(size_.y), style_->textPadding_.y);
    target.draw(style_->text_, states);
}

}
