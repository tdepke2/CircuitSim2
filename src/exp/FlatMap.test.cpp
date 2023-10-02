#include <FlatMap.h>

#include <iostream>
#include <map>
#include <spdlog/spdlog.h>
#include <string>

int main() {
    spdlog::info("Started FlatMap test.");

    std::map<int, std::string> stuff;
    stuff.key_comp();

    std::less<int> comp;
    spdlog::info("1<2 is {}, 4<3 is {}", comp(1, 2), comp(4, 3));

    std::map<int, std::string> m;
    m.insert(std::make_pair(1, "hello"));
    m.insert({std::make_pair(2, "world"), std::make_pair(3, "test")});
    m.emplace(4, "...");

    FlatMap<int, std::string> flat;
    flat.insert(std::make_pair(1, "hello"));
    flat.insert({5, "test"});
    flat.insert(std::make_pair(3, "is"));
    flat.insert(std::make_pair(2, "this"));
    flat.insert(std::make_pair(4, "a"));

    flat.insert(std::make_pair(3, "isn\'t"));
    flat.insert(std::make_pair(-3, "start"));
    flat.insert(std::make_pair(10, "end"));

    size_t i = 0;
    for (const auto& x : flat.debugGetVec()) {
        std::cout << "[" << i << "] " << x.first << " -> [" << x.second << "]\n";
        ++i;
    }

    return 0;
}
