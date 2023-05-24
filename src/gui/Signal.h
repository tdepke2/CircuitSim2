#pragma once

#include <cassert>
#include <functional>
#include <type_traits>
#include <map>

namespace gui {

// Would be nice to use an anonymous namespace, but that implies internal linkage.
namespace priv {
    extern unsigned int signalIdCounter;
}

/**
 * Stores a std::function with or without parameters (using a tagged union).
 * The function represents a callback, the parameters are optional in case the
 * user doesn't need to use them. This could also be implemented with
 * std::variant, but we'll keep it c++11 compatible.
 * 
 * This class originally had some memory allocation problems (wasn't using
 * placement new), I got some improvements from:
 * https://www.reddit.com/r/cpp_questions/comments/zm1j4l/an_stdvariant_alternative_for_c11/
 */
template<typename... Targs>
class Callback final {
public:
    /**
     * The union tag, corresponds to the current function type stored.
     */
    enum class Tag {
        withArgs, noArgs
    };

    /**
     * Constructor for function with parameters.
     * 
     * @param func Function-like object.
     */
    Callback(std::function<void(Targs...)> func) noexcept : withArgs_(std::move(func)), tag_(Tag::withArgs) {}

    /**
     * Constructor for function without parameters.
     * 
     * @param func Function-like object.
     */
    Callback(std::function<void()> func) noexcept : noArgs_(std::move(func)), tag_(Tag::noArgs) {}

    /**
     * Destructor.
     */
    ~Callback() {
        if (tag_ == Tag::withArgs) {
            withArgs_.~function();
        } else {
            noArgs_.~function();
        }
    }

    /**
     * Copy constructor.
     */
    Callback(const Callback& other) noexcept : tag_(other.tag_) {
        if (other.tag_ == Tag::withArgs) {
            ::new(&withArgs_) auto(other.withArgs_);
        } else {
            ::new(&noArgs_) auto(other.noArgs_);
        }
    }

    /**
     * Copy/move assignment.
     */
    Callback& operator=(Callback other) noexcept {
        swap(*this, other);
        return *this;
    }

    /**
     * Move constructor.
     */
    Callback(Callback&& other) noexcept : tag_(other.tag_) {
        if (other.tag_ == Tag::withArgs) {
            ::new(&withArgs_) auto(std::move(other.withArgs_));
        } else {
            ::new(&noArgs_) auto(std::move(other.noArgs_));
        }
    }

    /**
     * Swaps the contents of two `Callback` objects (copy-and-swap idiom).
     */
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

    /**
     * Get the current tag (type) of the union.
     * This should be used with `value()` to get the current value of the union,
     * such as `value<Tag::withArgs>()` if the tag is `Tag::withArgs`.
     */
    Tag getTag() const {
        return tag_;
    }

    /**
     * Get the value of the union as a function with parameters.
     */
    template<Tag tag, typename = typename std::enable_if<tag == Tag::withArgs>::type>
    const std::function<void(Targs...)>& value() const {
        assert(tag_ == Tag::withArgs);
        return withArgs_;
    }

    /**
     * Get the value of the union as a function without parameters.
     */
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


/**
 * Tracks `Callback` functions bound to a specific event.
 * A `Signal` represents a type of event that can occur, where the event may
 * need to invoke some immediate behavior (like a button press). This uses
 * template parameter pack to set the parameters for each `Callback`. Note that
 * at least one parameter is required in the template instance.
 */
template<typename... Targs>
class Signal {
public:
    /**
     * Default constructor.
     */
    Signal() : enabled_(true) {}

    /**
     * Set enable state.
     * A disabled `Signal` does not invoke any callbacks during `emit()`.
     */
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }

    /**
     * Check if enabled.
     */
    bool isEnabled() const {
        return enabled_;
    }

    /**
     * Connect a function-like object to the `Signal`.
     * The function can be anything convertible to a `Callback` (function
     * pointer, lambda expression, result from `std::bind()`, etc). The function
     * must return void and either have no parameters, or match the template
     * parameters of this `Signal`.
     * 
     * @param func Function-like object.
     * @return Unique handle to identify the function for disconnecting later.
     */
    template<typename F>
    unsigned int connect(F&& func) {
        // Using perfect forwarding has the drawback of creating very long error
        // messages when types don't line up. Verify the argument type here
        // before continuing.
        static_assert(
            std::is_constructible<Callback<Targs...>, F>::value,
            "Cannot convert argument \'func\' to a Callback, check the function signature."
        );

        while (callbacks_.count(priv::signalIdCounter)) {
            ++priv::signalIdCounter;
        }
        callbacks_.emplace(priv::signalIdCounter, std::forward<F>(func));

        return priv::signalIdCounter++;
    }

    /**
     * Removes a previously connected function from the `Signal`.
     * 
     * @param id Unique handle returned from `connect()`.
     * @return True if function was disconnected, false if not found.
     */
    bool disconnect(unsigned int id) {
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            callbacks_.erase(it);
            return true;
        }
        return false;
    }

    /**
     * Removes all connected functions.
     */
    void disconnectAll() {
        callbacks_.clear();
    }

    /**
     * Calls all of the connected functions.
     * The functions are called in the same order they were connected in (until
     * the handle returned from `connect()` overflows an unsigned int and wraps
     * back around).
     * 
     * @param funcArgs Arguments passed to each function if the function accepts
     * them. Must be provided even if all connected functions have no
     * parameters.
     */
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
    std::map<unsigned int, Callback<Targs...>> callbacks_;
};

} // namespace gui
