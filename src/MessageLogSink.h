#pragma once

#include <gui/widgets/ChatBox.h>

#include <array>
#include <memory>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <spdlog/details/null_mutex.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <string>
#include <utility>
#include <vector>

template<typename Mutex>
class MessageLogSink : public spdlog::sinks::base_sink<Mutex> {
public:
    MessageLogSink() :
        chatBox_(),
        bufferedMessages_(),
        styles_() {

        styles_[spdlog::level::trace] = {sf::Color::White, sf::Text::Regular};
        styles_[spdlog::level::debug] = {sf::Color::Cyan, sf::Text::Regular};
        styles_[spdlog::level::info] = {sf::Color::Green, sf::Text::Regular};
        styles_[spdlog::level::warn] = {sf::Color::Yellow, sf::Text::Regular};
        styles_[spdlog::level::err] = {sf::Color::Red, sf::Text::Regular};
        styles_[spdlog::level::critical] = {sf::Color::Red, sf::Text::Italic};
        styles_[spdlog::level::off] = {sf::Color::White, sf::Text::Regular};
    }

    void registerChatBox(std::shared_ptr<gui::ChatBox> chatBox) {
        if (chatBox != nullptr) {
            for (const auto& msg : bufferedMessages_) {
                chatBox->addLines(msg.first, styles_[msg.second].first, styles_[msg.second].second);
            }
            bufferedMessages_.clear();
        }
        chatBox_ = chatBox;
    }

protected:
    virtual void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        if (auto chatBox = chatBox_.lock()) {
            chatBox->addLines(fmt::to_string(formatted), styles_[msg.level].first, styles_[msg.level].second);
        } else {
            bufferedMessages_.emplace_back(fmt::to_string(formatted), msg.level);
        }
    }
    virtual void flush_() override {
        // Nothing to flush for this sink.
    }

private:
    std::weak_ptr<gui::ChatBox> chatBox_;
    std::vector<std::pair<std::string, spdlog::level::level_enum>> bufferedMessages_;
    std::array<std::pair<sf::Color, uint32_t>, spdlog::level::n_levels> styles_;
};

using MessageLogSinkMt = MessageLogSink<std::mutex>;
using MessageLogSinkSt = MessageLogSink<spdlog::details::null_mutex>;
