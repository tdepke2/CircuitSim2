#include <FlatMap.h>

#include <iostream>
#include <map>
#include <spdlog/spdlog.h>
#include <string>

template<typename T>
struct CustomComp {
    CustomComp() {
        std::cout << "CustomComp::CustomComp()\n";
    }
    inline bool operator()(const T& lhs, const T& rhs) {
        return lhs > rhs;
    }
};

template<typename Key, typename Value, typename Compare>
std::ostream& operator<<(std::ostream& out, const std::map<Key, Value, Compare>& map) {
    out << "{ ";
    for (const auto& v : map) {
        out << "[" << v.first << "] -> " << v.second << ", ";
    }
    return out << "}\n";
}

template<typename Key, typename Value, typename Compare>
std::ostream& operator<<(std::ostream& out, const FlatMap<Key, Value, Compare>& map) {
    out << "{ ";
    for (const auto& v : map) {
        out << "[" << v.first << "] -> " << v.second << ", ";
    }
    return out << "}\n";
}

struct Point {
    double x, y;
    friend std::ostream& operator<<(std::ostream& out, Point pt) {
        return out << "(" << pt.x << ", " << pt.y << ")";
    }
};

struct PointCmp {
    bool operator()(const Point& lhs, const Point& rhs) const {
        return lhs.x < rhs.x;
    }
};

template<template<typename...> class Map>
void testCtor() {
    // Modified example code from en.cppreference.com

    // Default constructor.
    Map<std::string, int> map1;
    map1["something"] = 69;
    map1["anything"] = 199;
    map1["that thing"] = 50;
    map1["another thing"] = 123;
    std::cout << "default ctor = " << map1 << "\n";

    // Range constructor.
    Map<std::string, int> range1(map1.find("anything"), map1.end());
    std::cout << "range ctor = " << range1 << "\n";

    Map<std::string, int> range2(map1.rbegin(), map1.rend());
    std::cout << "range ctor (reversed) = " << range2 << "\n";

    // Copy constructor.
    Map<std::string, int> copied1(map1);
    std::cout << "copy ctor = " << copied1 << "\n";

    // Move constructor.
    Map<std::string, int> moved1(std::move(map1));
    std::cout << "move ctor = " << moved1 << "\n";

    // Initializer list constructor.
    const Map<std::string, int> init1 {
        {"this", 100},
        {"can", 100},
        {"be", 100},
        {"const", 100}
    };
    std::cout << "init list ctor = " << init1 << "\n";

    const Map<std::string, int> init2 {
        {"hello", 1},
        {"this", 2},
        {"is", 3},
        {"a", 4},
        {"this", 6},
        {"test", 5}
    };
    std::cout << "init list ctor (dupes) = " << init2 << "\n";

    const Map<std::string, int> init3 {
        {"hello", 5},
        {"hello", 4},
        {"hello", 3},
        {"hello", 2},
        {"hello", 1}
    };
    std::cout << "init list ctor (lot of dupes) = " << init3 << "\n";

    // Custom comparator.
    Map<Point, double, PointCmp> magnitudes1 = {
        {{4, 3}, 5},
        {{5, -12}, 13},
        {{3, 4}, 5},
        {{-8, -15}, 17}
    };
    std::cout << "custom comp = " << magnitudes1 << "\n";

    // This lambda sorts points according to their magnitudes.
    auto cmpLambda = [&magnitudes1](const Point& lhs, const Point& rhs) {
        return magnitudes1[lhs] < magnitudes1[rhs];
    };
    Map<Point, double, decltype(cmpLambda)> magnitudes2({
        {{4, 3}, 5}
    }, cmpLambda);

    magnitudes2.insert(std::pair<Point, double>({5, -12}, 13));
    magnitudes2.insert({{3, 4}, 5});
    magnitudes2.insert({Point{-8.0, -15.0}, 17});
    std::cout << "custom lambda comp = " << magnitudes2 << "\n";
}

template<template<typename...> class Map>
void testAccess() {
    Map<std::string, std::string> myMap;
    std::cout << "empty myMap = " << myMap << "\n";

    try {
        std::cout << "  item [test_at] = " << myMap.at("test_at") << "\n";
    } catch (std::out_of_range& ex) {
        std::cout << "  item [test_at] exception: " << ex.what() << "\n";
    }
    std::cout << "  item [test_op_brackets] = " << myMap["test_op_brackets"] << "\n";
    myMap["new"] = "element";
    myMap["another"];
    myMap.at("another") = "element!";
    std::cout << "modified myMap = " << myMap << "\n";

    const Map<std::string, std::string> myMap2 {
        {"hello", "test"},
        {"numbers", "123"}
    };
    std::cout << "  item [hello] = " << myMap2.at("hello") << "\n";
    // Below should not compile (brackets operator isn't const).
    //std::cout << "  item [numbers] = " << myMap2["numbers"] << "\n";
    std::cout << "myMap2 = " << myMap2 << "\n";

    Map<std::string, int> wordMap;
    auto words = {
        "this", "sentence", "is", "not", "a", "sentence", "this", "sentence", "is", "a", "hoax"
    };
    for (const auto& w : words) {
        ++wordMap[w];
    }
    wordMap["none"];
    std::cout << "wordMap = " << wordMap << "\n";
}

int main() {
    spdlog::info("Started FlatMap test.");

    std::map<int, std::string> stuff;
    stuff.key_comp();

    /*std::less<int> comp;
    spdlog::info("1<2 is {}, 4<3 is {}", comp(1, 2), comp(4, 3));

    std::map<int, std::string> m;
    m.insert(decltype(m)::value_type(1, "hello"));
    m.insert({decltype(m)::value_type(2, "world"), decltype(m)::value_type(3, "test")});
    m.emplace(4, "...");

    FlatMap<int, std::string, CustomComp<int>> flat;
    auto val = decltype(flat)::value_type(1, "hello");
    flat.insert(val);
    flat.insert({5, "test"});
    flat.insert(decltype(flat)::value_type(3, "is"));
    flat.insert(decltype(flat)::value_type(2, "this"));
    flat.insert(decltype(flat)::value_type(4, "a"));

    flat.emplace(3, "isn\'t");
    flat.emplace(-3, "start");
    flat.emplace(10, "end");

    m.at(1) = "cool";
    flat.at(1) = "cool";

    size_t i = 0;
    for (const auto& x : flat.debugGetVec()) {
        std::cout << "[" << i << "] " << x.first << " -> [" << x.second << "]\n";
        ++i;
    }*/

    std::cout << "\n\ntestCtor<std::map>()\n";
    testCtor<std::map>();
    std::cout << "\ntestCtor<FlatMap>()\n";
    testCtor<FlatMap>();

    std::cout << "\n\ntestAccess<std::map>()\n";
    testAccess<std::map>();
    std::cout << "\ntestAccess<FlatMap>()\n";
    testAccess<FlatMap>();

    return 0;
}
