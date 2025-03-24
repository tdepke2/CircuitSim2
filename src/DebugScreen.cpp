#include <DebugScreen.h>
#include <MakeUnique.h>

#include <algorithm>
#include <cassert>
#include <spdlog/spdlog.h>

std::unique_ptr<DebugScreen> DebugScreen::instance_;

void DebugScreen::init(const sf::Font& font, unsigned int characterSize, const sf::Vector2u& windowSize) {
    assert(!instance_);
    instance_ = details::make_unique<DebugScreen>(Private(), font, characterSize, windowSize);
}

DebugScreen* DebugScreen::instance() {
    return instance_.get();
}

struct DebugScreen::NamedTexture {
    std::string name;
    const sf::Texture* texture;

    NamedTexture(std::string name, const sf::Texture* texture) :
        name(name),
        texture(texture) {
    }
};

DebugScreen::DebugScreen(Private, const sf::Font& font, unsigned int characterSize, const sf::Vector2u& windowSize) :
    mode_(Mode::def),
    modeStates_(),
    visible_(false),
    font_(font),
    characterSize_(characterSize),
    windowSize_(windowSize),
    nextFieldPos_(2.0f, 0.0f),
    fields_(),
    customFields_(),
    textures_(),
    profilerEvents_() {

    modeStates_.fill(0);
    fields_.reserve(static_cast<size_t>(Field::count));
    for (size_t i = 0; i < static_cast<size_t>(Field::count); ++i) {
        fields_.emplace_back(addField(false));
    }
}

bool DebugScreen::processEvent(const sf::Event& event) {
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == 'p') {
            spdlog::info("Captured trace ({} events):", profilerEvents_.size());
            decltype(profilerEvents_.begin()->second) firstTimePoint;
            while (!profilerEvents_.empty()) {
                auto minTimeEvent = profilerEvents_.begin();
                for (auto profilerEvent = profilerEvents_.begin(); profilerEvent != profilerEvents_.end(); ++profilerEvent) {
                    if (profilerEvent->second < minTimeEvent->second) {
                        minTimeEvent = profilerEvent;
                    }
                }
                if (firstTimePoint.time_since_epoch().count() == 0) {
                    firstTimePoint = minTimeEvent->second;
                }
                spdlog::info("  {} at time {}ms", minTimeEvent->first, std::chrono::duration<double, std::milli>(minTimeEvent->second - firstTimePoint).count());
                profilerEvents_.erase(minTimeEvent);
            }
        } else {
            return false;
        }
        return true;
    } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F3) {
            visible_ = !visible_;
        } else if (event.key.code == sf::Keyboard::Up) {
            mode_ = static_cast<Mode>((static_cast<int>(mode_) + static_cast<int>(Mode::count) - 1) % static_cast<int>(Mode::count));
        } else if (event.key.code == sf::Keyboard::Down) {
            mode_ = static_cast<Mode>((static_cast<int>(mode_) + 1) % static_cast<int>(Mode::count));
        } else if (event.key.code == sf::Keyboard::Left) {
            --modeStates_[static_cast<int>(mode_)];
        } else if (event.key.code == sf::Keyboard::Right) {
            ++modeStates_[static_cast<int>(mode_)];
        } else {
            return false;
        }
        return true;
    } else if (event.type == sf::Event::Resized) {
        windowSize_.x = event.size.width;
        windowSize_.y = event.size.height;
    }
    return false;
}

DebugScreen::Mode DebugScreen::getMode() const {
    return mode_;
}

std::string DebugScreen::getModeString() const {
    switch (mode_) {
    case Mode::def:
        return "default";
    default:
        return "texture " + std::to_string(modeStates_[static_cast<int>(mode_)]);
    }
}

void DebugScreen::setVisible(bool visible) {
    visible_ = visible;
}

bool DebugScreen::isVisible() const {
    return visible_;
}

void DebugScreen::setBorders(const sf::Vector2i& topLeft, const sf::Vector2i& bottomRight) {
    borderTopLeft_ = topLeft;
    borderBottomRight_ = bottomRight;
}

sf::Text& DebugScreen::getField(Field field) {
    return fields_[static_cast<size_t>(field)];
}

sf::Text& DebugScreen::getField(const std::string& customName) {
    auto custom = customFields_.find(customName);
    if (custom != customFields_.end()) {
        return custom->second;
    }
    return customFields_.emplace(customName, addField(true)).first->second;
}

void DebugScreen::registerTexture(const std::string& name, const sf::Texture* texture) {
    for (auto& t : textures_) {
        if (t.name == name) {
            if (t.texture != texture) {
                spdlog::error("Registered different texture for existing texture \"{}\".", name);
                t.texture = texture;
            }
            return;
        }
    }
    textures_.emplace_back(name, texture);
}

void DebugScreen::profilerEvent(const std::string& name) {
    profilerEvents_[name] = std::chrono::high_resolution_clock::now();
}

sf::Text DebugScreen::addField(bool isCustom) {
    sf::Text text("", font_, characterSize_);
    text.setPosition(nextFieldPos_);
    nextFieldPos_.y += 1.0f * characterSize_;
    if (!isCustom) {
        text.setFillColor(sf::Color::White);
    } else {
        text.setFillColor(sf::Color::Cyan);
    }
    return text;
}

void DebugScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!visible_) {
        return;
    }

    sf::Vector2i clippedSize = {
        std::max(static_cast<int>(windowSize_.x) - borderTopLeft_.x - borderBottomRight_.x, 0),
        std::max(static_cast<int>(windowSize_.y) - borderTopLeft_.y - borderBottomRight_.y, 0),
    };
    states.transform.translate(static_cast<sf::Vector2f>(borderTopLeft_));

    if (mode_ == Mode::def) {
        for (const auto& field : fields_) {
            target.draw(field, states);
        }
        for (const auto& custom : customFields_) {
            target.draw(custom.second, states);
        }
    } else if (mode_ == Mode::textures) {
        sf::Text textureField("Texture: none", font_, characterSize_);
        textureField.setPosition(fields_[0].getPosition() + sf::Vector2f(0.0f, 1.0f * characterSize_));
        if (modeStates_[static_cast<int>(mode_)] >= 0 && modeStates_[static_cast<int>(mode_)] < static_cast<int>(textures_.size())) {
            const NamedTexture& namedTex = textures_[modeStates_[static_cast<int>(mode_)]];
            textureField.setString("Texture: " + namedTex.name + " (size " + std::to_string(namedTex.texture->getSize().x) + " by " + std::to_string(namedTex.texture->getSize().y) + ")");

            sf::Sprite sprite(*namedTex.texture);
            sprite.setPosition(0.0f, textureField.getPosition().y + 1.0f * characterSize_ + 4.0f);
            float scaleX = (clippedSize.x - sprite.getPosition().x) / namedTex.texture->getSize().x;
            float scaleY = (clippedSize.y - sprite.getPosition().y) / namedTex.texture->getSize().y;
            float scale = std::min(scaleX, scaleY);
            sprite.setScale(scale, scale);
            target.draw(sprite, states);
        }

        target.draw(fields_[0], states);
        target.draw(textureField, states);
    }
}
