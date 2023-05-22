#pragma once

#include <cassert>
#include <functional>
#include <type_traits>
#include <unordered_map>

namespace gui {

// Would be nice to use an anonymous namespace, but that implies internal linkage.
namespace priv {
    extern unsigned int signalIdCounter;
}

// Uses a tagged union to store the function object corresponding to a callback.
// Could also use std::variant here, but we'll keep it c++11 compatible.
// This class originally had some memory allocation problems (wasn't using placement new), I got some improvements from:
// https://www.reddit.com/r/cpp_questions/comments/zm1j4l/an_stdvariant_alternative_for_c11/
template<typename... Targs>
class Callback final {
public:
    enum class Tag {
        withArgs, noArgs
    };
    Callback(std::function<void(Targs...)> func) noexcept : withArgs_(std::move(func)), tag_(Tag::withArgs) {}
    Callback(std::function<void()> func) noexcept : noArgs_(std::move(func)), tag_(Tag::noArgs) {}
    ~Callback() {
        if (tag_ == Tag::withArgs) {
            withArgs_.~function();
        } else {
            noArgs_.~function();
        }
    }
    Callback(const Callback& other) noexcept : tag_(other.tag_) {
        if (other.tag_ == Tag::withArgs) {
            ::new(&withArgs_) auto(other.withArgs_);
        } else {
            ::new(&noArgs_) auto(other.noArgs_);
        }
    }
    Callback& operator=(Callback other) noexcept {
        swap(*this, other);
        return *this;
    }
    Callback(Callback&& other) noexcept : tag_(other.tag_) {
        if (other.tag_ == Tag::withArgs) {
            ::new(&withArgs_) auto(std::move(other.withArgs_));
        } else {
            ::new(&noArgs_) auto(std::move(other.noArgs_));
        }
    }
    friend void swap(Callback& a, Callback& b) noexcept {
        using std::swap;
        if (a.tag_ != b.tag_) {
            // If types don't line up, just force a reallocation for each object.
            Callback<Targs...> temp = std::move(a);
            a.~Callback();
            ::new(&a) Callback(std::move(b));
            b.~Callback();
            ::new(&b) Callback(std::move(temp));
        } else if (a.tag_ == Tag::withArgs) {
            swap(a.withArgs_, b.withArgs_);
        } else {
            swap(a.noArgs_, b.noArgs_);
        }
    }

    Tag getTag() const {
        return tag_;
    }

    template<Tag tag, typename = typename std::enable_if<tag == Tag::withArgs>::type>
    const std::function<void(Targs...)>& value() const {
        assert(tag_ == Tag::withArgs);
        return withArgs_;
    }

    template<Tag tag, typename = typename std::enable_if<tag == Tag::noArgs>::type>
    const std::function<void()>& value() const {
        assert(tag_ == Tag::noArgs);
        return noArgs_;
    }

private:
    union {
        std::function<void(Targs...)> withArgs_;
        std::function<void()> noArgs_;
    };
    Tag tag_;
};

template<typename... Targs>
class Signal {
public:
    Signal() : enabled_(true) {}

    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }

    bool isEnabled() const {
        return enabled_;
    }

    template<typename F>
    unsigned int connect(F&& func) {
        while (callbacks_.count(priv::signalIdCounter)) {
            ++priv::signalIdCounter;
        }

        // Using perfect forwarding has the drawback of creating very long error
        // messages when types don't line up. Verify the argument type here
        // before continuing.
        static_assert(
            std::is_constructible<Callback<Targs...>, F>::value,
            "Cannot convert argument \'func\' to a Callback, check the function signature."
        );
        callbacks_.emplace(priv::signalIdCounter, std::forward<F>(func));

        return priv::signalIdCounter++;
    }
    bool disconnect(unsigned int id) {
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            callbacks_.erase(it);
            return true;
        }
        return false;
    }
    void disconnectAll() {
        callbacks_.clear();
    }
    void emit(Targs... funcArgs) const {
        if (!enabled_) {
            return;
        }
        using Tag = typename Callback<Targs...>::Tag;
        for (const auto& c : callbacks_) {
            if (c.second.getTag() == Tag::withArgs) {
                c.second.template value<Tag::withArgs>()(funcArgs...);
            } else {
                c.second.template value<Tag::noArgs>()();
            }
        }
    }

private:
    bool enabled_;
    std::unordered_map<unsigned int, Callback<Targs...>> callbacks_;
};

}
