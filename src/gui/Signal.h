#pragma once

#include <functional>



#include <iostream>




#include <unordered_map>

namespace gui {

/*
class Signal {
public:
    unsigned int connect(std::function<void()> callback);
    bool disconnect(unsigned int id);
    void disconnectAll();
    void emit() const;

protected: // FIXME private?
    static unsigned int idCounter_;

    std::unordered_map<unsigned int, std::function<void()>> callbacks_;
};

template <typename T>
class SignalT : public Signal {
public:
    void emit(T t1) const {
        for (const auto& c : callbacks_) {
            c.second(t1);
        }
    }
};*/

/**/
// really should go in a private namespace...
extern int counter;

// mock of std::function for testing
template<typename T>
class function {
public:
    function()   { id = counter++; std::cout << "  function<" << typeid(T).name() << "> default ctor " << id << "\n"; }
    function(T)  { id = counter++; std::cout << "  function<" << typeid(T).name() << "> T ctor " << id << "\n"; }
    ~function()                  { std::cout << "  function<" << typeid(T).name() << "> dtor " << id << "\n"; }
    function(const function& o)  {
                   id = counter++; std::cout << "  function<" << typeid(T).name() << "> copy ctor " << id << " (from " << o.id << ")\n";
    }
    function& operator=(const function& o) {
                   id = counter++; std::cout << "  function<" << typeid(T).name() << "> copy assign " << id << " (from " << o.id << ")\n";
                   return *this;
    }
    function(function&& o) {
                        id = o.id; std::cout << "  function<" << typeid(T).name() << "> move ctor " << id << "\n";
    }
    function& operator=(function&& o) {
                        id = o.id; std::cout << "  function<" << typeid(T).name() << "> move assign " << id << "\n";
                        return *this;
    }
    template<typename... Args>
    void operator()(Args...) {
                                   std::cout << "  function<" << typeid(T).name() << "> operator() " << id << "\n";
    }
    int id;
};//*/


// Uses a tagged union to store the function object corresponding to a callback.
// Could also use std::variant here, but we'll keep it c++11 compatible.
// This class originally had some memory allocation problems (wasn't using placement new), I got some improvements from:
// https://www.reddit.com/r/cpp_questions/comments/zm1j4l/an_stdvariant_alternative_for_c11/
template<typename... Targs>
class Callback final {
public:
    union {
        std::function<void(Targs...)> withArgs;
        std::function<void()> noArgs;
    };
    enum class Type {
        withArgs, noArgs
    } type;
    Callback(std::function<void(Targs...)> func) : withArgs(std::move(func)), type(Type::withArgs) { std::cout << "Callback(std::function<void(Targs...)>\n"; }
    Callback(std::function<void()> func) : noArgs(std::move(func)), type(Type::noArgs) { std::cout << "Callback(std::function<void()>\n"; }
    ~Callback() {
        if (type == Type::withArgs) {
            std::cout << "~Callback() with args\n";
            withArgs.~function();
        } else {
            std::cout << "~Callback() no args\n";
            noArgs.~function();
        }
    }
    Callback(const Callback& other) : type(other.type) {
        if (other.type == Type::withArgs) {
            std::cout << "Callback(const Callback& other) with args\n";
            ::new(&withArgs) auto(other.withArgs);
        } else {
            std::cout << "Callback(const Callback& other) no args\n";
            ::new(&noArgs) auto(other.noArgs);
        }
    }
    Callback& operator=(Callback other) {
        if (other.type == Type::withArgs) {
            std::cout << "Callback& operator=(Callback other) with args\n";
        } else {
            std::cout << "Callback& operator=(Callback other) no args\n";
        }
        swap(*this, other);
        return *this;
    }
    Callback(Callback&& other) noexcept : type(other.type) {
        if (other.type == Type::withArgs) {
            std::cout << "Callback(Callback&& other) with args\n";
            ::new(&withArgs) auto(std::move(other.withArgs));
        } else {
            std::cout << "Callback(Callback&& other) no args\n";
            ::new(&noArgs) auto(std::move(other.noArgs));
        }
    }
    //Callback& operator=(Callback&& other) = delete;

    friend void swap(Callback& a, Callback& b) noexcept {
        using std::swap;
        if (a.type != b.type) {
            std::cout << "swap() with different Callback types\n";
            // If types don't line up, just force a reallocation for each object.
            Callback<Targs...> temp = std::move(a);
            a.~Callback();
            ::new(&a) Callback(std::move(b));
            b.~Callback();
            ::new(&b) Callback(std::move(temp));
        } else if (a.type == Type::withArgs) {
            std::cout << "swap() withArgs\n";
            swap(a.withArgs, b.withArgs);
        } else {
            std::cout << "swap() noArgs\n";
            swap(a.noArgs, b.noArgs);
        }
    }
};

template<typename... Targs>
class Signal {
public:
    template<typename F>
    unsigned int connect(F&& func) {
        //return connectImpl(Callback<Targs...>(std::move(f)));
        while (callbacks_.count(idCounter_)) {
            ++idCounter_;
        }


        // FIXME: put a check against a bad func signature.


        callbacks_.emplace(idCounter_, std::forward<F>(func));

        return idCounter_++;
    }
    /*unsigned int connect(const std::function<void(Targs...)>& func) {
        Callback<Targs...> cb(func);
        return connectImpl(cb);
    }
    unsigned int connect(const std::function<void()>& func) {
        Callback<Targs...> cb(func);
        return connectImpl(cb);
    }*/
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
        for (const auto& c : callbacks_) {
            if (c.second.type == Callback<Targs...>::Type::withArgs) {
                c.second.withArgs(funcArgs...);
            } else {
                c.second.noArgs();
            }
        }
    }

private:
    static unsigned int idCounter_;  // FIXME don't use static member for template?

    /*unsigned int connectImpl(const Callback<Targs...>& callback) {
        while (callbacks_.count(idCounter_)) {
            ++idCounter_;
        }
        std::cout << "time to emplace...\n";
        callbacks_.emplace(idCounter_, callback);
        std::cout << "emplace done\n";

        return idCounter_++;
    }*/

    std::unordered_map<unsigned int, Callback<Targs...>> callbacks_;
};

template<typename... Targs>
unsigned int Signal<Targs...>::idCounter_ = 0;

}
