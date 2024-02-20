#include <FlatMap.h>

#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>

template<typename Key, typename Value, typename Compare>
std::ostream& operator<<(std::ostream& out, const std::map<Key, Value, Compare>& map) {
    out << "{ ";
    size_t i = 0;
    for (const auto& v : map) {
        out << (i++ == 0 ? "" : ", ") << "[" << v.first << "] -> " << v.second;
    }
    return out << " }";
}

template<typename Key, typename Value, typename Compare>
std::ostream& operator<<(std::ostream& out, const FlatMap<Key, Value, Compare>& map) {
    out << "{ ";
    size_t i = 0;
    for (const auto& v : map) {
        out << (i++ == 0 ? "" : ", ") << "[" << v.first << "] -> " << v.second;
    }
    return out << " }";
}

#include <catch2/catch.hpp>


/**
 * Checks for range equality with a string comparison. This is definitely not
 * the best method to do this but it makes things easy.
 */
template<typename T>
class EqualsRange : public Catch::MatcherBase<T> {
public:
    EqualsRange(const std::string& expected) :
        expected_(expected) {
    }

    virtual bool match(const T& in) const override {
        std::ostringstream s;
        s << in;
        return s.str() == expected_;
    }

    virtual std::string describe() const override {
        std::ostringstream s;
        s << "\nEquals range:\n" << expected_;
        return s.str();
    }

private:
    std::string expected_;
};

TEMPLATE_TEST_CASE("Test ctor", "[FlatMap]", (std::map<std::string, int>), (FlatMap<std::string, int>)) {
    // Modified example code from en.cppreference.com

    // Default constructor.
    TestType map1;
    map1["something"] = 69;
    map1["anything"] = 199;
    map1["that thing"] = 50;
    map1["another thing"] = 123;
    //std::cout << "default ctor = " << map1 << "\n";
    REQUIRE_THAT(map1, EqualsRange<TestType>(
        "{ [another thing] -> 123, [anything] -> 199, [something] -> 69, [that thing] -> 50 }"
    ));

    // Range constructor.
    TestType range1(map1.find("anything"), map1.end());
    //std::cout << "range ctor = " << range1 << "\n";
    REQUIRE_THAT(range1, EqualsRange<TestType>(
        "{ [anything] -> 199, [something] -> 69, [that thing] -> 50 }"
    ));

    TestType range2(map1.rbegin(), map1.rend());
    //std::cout << "range ctor (reversed) = " << range2 << "\n";
    REQUIRE_THAT(range2, EqualsRange<TestType>(
        "{ [another thing] -> 123, [anything] -> 199, [something] -> 69, [that thing] -> 50 }"
    ));

    TestType range3(map1.begin(), map1.begin());
    //std::cout << "range ctor (empty) = " << range3 << "\n";
    REQUIRE_THAT(range3, EqualsRange<TestType>(
        "{  }"
    ));

    // Copy constructor.
    TestType copied1(map1);
    //std::cout << "copy ctor = " << copied1 << "\n";
    REQUIRE_THAT(copied1, EqualsRange<TestType>(
        "{ [another thing] -> 123, [anything] -> 199, [something] -> 69, [that thing] -> 50 }"
    ));

    // Move constructor.
    TestType moved1(std::move(map1));
    //std::cout << "move ctor = " << moved1 << "\n";
    REQUIRE_THAT(moved1, EqualsRange<TestType>(
        "{ [another thing] -> 123, [anything] -> 199, [something] -> 69, [that thing] -> 50 }"
    ));

    // Initializer list constructor.
    const TestType init1 {
        {"this", 100},
        {"can", 100},
        {"be", 100},
        {"const", 100}
    };
    //std::cout << "init list ctor = " << init1 << "\n";
    REQUIRE_THAT(init1, EqualsRange<TestType>(
        "{ [be] -> 100, [can] -> 100, [const] -> 100, [this] -> 100 }"
    ));

    const TestType init2 {
        {"hello", 1},
        {"this", 2},
        {"is", 3},
        {"a", 4},
        {"this", 6},
        {"test", 5}
    };
    //std::cout << "init list ctor (dupes) = " << init2 << "\n";
    REQUIRE_THAT(init2, EqualsRange<TestType>(
        "{ [a] -> 4, [hello] -> 1, [is] -> 3, [test] -> 5, [this] -> 2 }"
    ));

    const TestType init3 {
        {"hello", 5},
        {"hello", 4},
        {"hello", 3},
        {"hello", 2},
        {"hello", 1}
    };
    //std::cout << "init list ctor (lot of dupes) = " << init3 << "\n";
    REQUIRE_THAT(init3, EqualsRange<TestType>(
        "{ [hello] -> 5 }"
    ));
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
void testCtorCustomComp() {
    INFO("Map type is " << typeid(Map<Point, double, PointCmp>).name());

    // Custom comparator.
    Map<Point, double, PointCmp> magnitudes1 = {
        {{4, 3}, 5},
        {{5, -12}, 13},
        {{3, 4}, 5},
        {{-8, -15}, 17}
    };
    //std::cout << "custom comp = " << magnitudes1 << "\n";
    REQUIRE_THAT(magnitudes1, EqualsRange<decltype(magnitudes1)>(
        "{ [(-8, -15)] -> 17, [(3, 4)] -> 5, [(4, 3)] -> 5, [(5, -12)] -> 13 }"
    ));

    // This lambda sorts points according to their magnitudes.
    auto cmpLambda = [&magnitudes1](const Point& lhs, const Point& rhs) {
        return magnitudes1[lhs] < magnitudes1[rhs];
    };
    Map<Point, double, decltype(cmpLambda)> magnitudes2({
        {{4, 3}, 5}
    }, cmpLambda);
    REQUIRE_THAT(magnitudes2, EqualsRange<decltype(magnitudes2)>(
        "{ [(4, 3)] -> 5 }"
    ));

    magnitudes2.insert(std::pair<Point, double>({5, -12}, 13));
    magnitudes2.insert({{3, 4}, 5});
    magnitudes2.insert({Point{-8.0, -15.0}, 17});
    //std::cout << "custom lambda comp = " << magnitudes2 << "\n";
    REQUIRE_THAT(magnitudes2, EqualsRange<decltype(magnitudes2)>(
        "{ [(4, 3)] -> 5, [(5, -12)] -> 13, [(-8, -15)] -> 17 }"
    ));
}

TEST_CASE("Test ctor custom comparator", "[FlatMap]") {
    testCtorCustomComp<std::map>();
    testCtorCustomComp<FlatMap>();
}

template<template<typename...> class Map>
void testAccess() {
    INFO("Map type is " << typeid(Map<std::string, std::string>).name());

    Map<std::string, std::string> myMap;
    //std::cout << "empty myMap = " << myMap << "\n";
    REQUIRE_THAT(myMap, EqualsRange<decltype(myMap)>(
        "{  }"
    ));

    /*try {
        std::cout << "  item [test_at] = " << myMap.at("test_at") << "\n";
    } catch (std::out_of_range& ex) {
        std::cout << "  item [test_at] exception: " << ex.what() << "\n";
    }*/
    REQUIRE_THROWS_AS(myMap.at("test_at"), std::out_of_range);

    //std::cout << "  item [test_op_brackets] = " << myMap["test_op_brackets"] << "\n";
    REQUIRE(myMap["test_op_brackets"] == "");

    myMap["new"] = "element";
    myMap["another"];
    myMap.at("another") = "element!";
    //std::cout << "modified myMap = " << myMap << "\n";
    REQUIRE_THAT(myMap, EqualsRange<decltype(myMap)>(
        "{ [another] -> element!, [new] -> element, [test_op_brackets] ->  }"
    ));

    const Map<std::string, std::string> myMap2 {
        {"hello", "test"},
        {"numbers", "123"}
    };
    //std::cout << "  item [hello] = " << myMap2.at("hello") << "\n";
    REQUIRE(myMap2.at("hello") == "test");

    // Below should not compile (brackets operator isn't const).
    //std::cout << "  item [numbers] = " << myMap2["numbers"] << "\n";

    //std::cout << "myMap2 = " << myMap2 << "\n";
    REQUIRE_THAT(myMap2, EqualsRange<decltype(myMap2)>(
        "{ [hello] -> test, [numbers] -> 123 }"
    ));

    Map<std::string, int> wordMap;
    auto words = {
        "this", "sentence", "is", "not", "a", "sentence", "this", "sentence", "is", "a", "hoax"
    };
    for (const auto& w : words) {
        ++wordMap[w];
    }
    wordMap["none"];
    //std::cout << "wordMap = " << wordMap << "\n";
    REQUIRE_THAT(wordMap, EqualsRange<decltype(wordMap)>(
        "{ [a] -> 2, [hoax] -> 1, [is] -> 2, [none] -> 0, [not] -> 1, [sentence] -> 3, [this] -> 2 }"
    ));
}

TEST_CASE("Test access", "[FlatMap]") {
    testAccess<std::map>();
    testAccess<FlatMap>();
}

template<template<typename...> class Map>
void testIter() {
    INFO("Map type is " << typeid(Map<int, std::string>).name());

    Map<int, std::string> map1 {
        {101, "emet"},
        {56, "dolor"},
        {-4, "lorem"},
        {79, "sit"},
        {3, "ipsum"}
    };
    std::ostringstream map1Forward;
    for (auto it = map1.cbegin(); it != map1.cend(); ++it) {
        map1Forward << "[" << it->first << "] -> " << it->second << ", ";
    }
    REQUIRE(map1Forward.str() == "[-4] -> lorem, [3] -> ipsum, [56] -> dolor, [79] -> sit, [101] -> emet, ");

    std::ostringstream map1Reverse;
    for (auto it = map1.rbegin(); it != map1.rend(); ++it) {
        map1Reverse << "[" << it->first << "] -> " << it->second << ", ";
    }
    REQUIRE(map1Reverse.str() == "[101] -> emet, [79] -> sit, [56] -> dolor, [3] -> ipsum, [-4] -> lorem, ");

    const Map<int, std::string, std::greater<int>> map2 {
        {101, "emet"},
        {56, "dolor"},
        {-4, "lorem"},
        {79, "sit"},
        {3, "ipsum"}
    };
    std::ostringstream map2Forward;
    for (auto it = map2.begin(); it != map2.end(); ++it) {
        map2Forward << "[" << it->first << "] -> " << it->second << ", ";
    }
    REQUIRE(map2Forward.str() == "[101] -> emet, [79] -> sit, [56] -> dolor, [3] -> ipsum, [-4] -> lorem, ");

    std::ostringstream map2Reverse;
    for (auto it = map2.crbegin(); it != map2.crend(); ++it) {
        map2Reverse << "[" << it->first << "] -> " << it->second << ", ";
    }
    REQUIRE(map2Reverse.str() == "[-4] -> lorem, [3] -> ipsum, [56] -> dolor, [79] -> sit, [101] -> emet, ");

    Point points[3] = {{2, 0}, {1, 0}, {3, 0}};
    auto pointCmp = [](const Point* lhs, const Point* rhs) {
        return lhs->x < rhs->x;
    };
    Map<Point*, double, decltype(pointCmp)> magnitudes(
        {{points, 2}, {points + 1, 1}, {points + 2, 3}},
        pointCmp
    );

    // Change each y-coordinate from 0 to the magnitude.
    for (auto iter = magnitudes.begin(); iter != magnitudes.end(); ++iter) {
        auto p = iter->first;
        p->y = magnitudes[p];
    }

    // Update and print the magnitude of each node.
    std::ostringstream magnitudesStr;
    for (auto iter = magnitudes.begin(); iter != magnitudes.end(); ++iter) {
        auto p = iter->first;
        magnitudes[p] = std::hypot(p->x, p->y);
        magnitudesStr << "[(" << p->x << ", " << p->y << ")] -> " << iter->second << ", ";
    }
    REQUIRE(magnitudesStr.str() == "[(1, 1)] -> 1.41421, [(2, 2)] -> 2.82843, [(3, 3)] -> 4.24264, ");

    // Repeat the above with the range-based for loop.
    std::ostringstream magnitudesStr2;
    for (auto i : magnitudes) {
        auto p = i.first;
        p->y = i.second;
        magnitudes[p] = std::hypot(p->x, p->y);
        magnitudesStr2 << "[(" << p->x << ", " << p->y << ")] -> " << magnitudes[p] << ", ";
    }
    REQUIRE(magnitudesStr2.str() == "[(1, 1.41421)] -> 1.73205, [(2, 2.82843)] -> 3.4641, [(3, 4.24264)] -> 5.19615, ");
}

TEST_CASE("Test iteration", "[FlatMap]") {
    testIter<std::map>();
    testIter<FlatMap>();
}

template<typename Iter, typename Key, typename Value>
void requireInsert(const std::pair<Iter, bool>& result, const std::pair<Key, Value>& expectedNode, bool expectedStatus) {
    //std::cout << "check {{" << result.first->first << ", " << result.first->second << "}, " << result.second << "}\n";
    //std::cout << "matches {{" << expectedNode.first << ", " << expectedNode.second << "}, " << expectedStatus << "}\n";
    REQUIRE(result.first->first == expectedNode.first);
    REQUIRE(result.first->second == expectedNode.second);
    REQUIRE(result.second == expectedStatus);
}

template<template<typename...> class Map>
void testModify() {
    INFO("Map type is " << typeid(Map<std::string, float>).name());

    Map<std::string, float> heights;
    REQUIRE(heights.size() == 0);
    REQUIRE(heights.empty());

    // Insert from rvalue reference.
    const auto insertResult = heights.insert({"Hinata", 162.8f});
    requireInsert(insertResult, std::pair<std::string, float>("Hinata", 162.8f), true);
    REQUIRE(heights.size() == 1);
    REQUIRE_FALSE(heights.empty());
    REQUIRE(heights.find("Hinata") == insertResult.first);
    REQUIRE(heights.find("Kageyama") == heights.end());

    // Insert from lvalue reference.
    const auto insertResult2 = heights.insert(*insertResult.first);
    requireInsert(insertResult2, *insertResult.first, false);
    REQUIRE(heights.size() == 1);
    REQUIRE_FALSE(heights.empty());

    std::pair<std::string, float> nodeAzumane("Azumane", 184.7f);
    const auto insertResult3 = heights.insert(nodeAzumane);
    requireInsert(insertResult3, nodeAzumane, true);
    REQUIRE(heights.size() == 2);
    REQUIRE_FALSE(heights.empty());

    // Insert via forwarding to emplace.
    const auto insertResult4 = heights.insert(std::pair<std::string, float>{"Kageyama", 180.6f});
    requireInsert(insertResult4, std::pair<std::string, float>("Kageyama", 180.6f), true);
    REQUIRE(heights.size() == 3);
    REQUIRE_FALSE(heights.empty());

    // Insert from iterator range.
    Map<std::string, float> heightsCopy;
    heightsCopy.insert(std::begin(heights), std::begin(heights));
    REQUIRE(heightsCopy.size() == 0);
    REQUIRE(heightsCopy.empty());
    heightsCopy.insert(std::begin(heights), std::end(heights));
    REQUIRE(heightsCopy.size() == 3);
    REQUIRE_FALSE(heightsCopy.empty());

    // Insert from initializer_list.
    heightsCopy.insert({{"Kozume", 169.2f}, {"Kuroo", 187.7f}});
    REQUIRE(heightsCopy.size() == 5);
    REQUIRE_FALSE(heightsCopy.empty());

    heightsCopy.insert({{"Tsukishima", 188.3f}, {"Kageyama", -99.8f}});
    REQUIRE(heightsCopy.size() == 6);
    REQUIRE_FALSE(heightsCopy.empty());

    heightsCopy.insert({{"Azumane", 0.0f}, {"Azumane", 0.0f}, {"Azumane", 0.0f}});
    REQUIRE(heightsCopy.size() == 6);
    REQUIRE_FALSE(heightsCopy.empty());

    // Insert partial duplicates, and a full range.
    heights.insert(heightsCopy.begin(), heightsCopy.end());
    REQUIRE(heights.size() == 6);
    REQUIRE_FALSE(heights.empty());
    heights.clear();
    REQUIRE(heights.size() == 0);
    REQUIRE(heights.empty());
    heights.insert(heightsCopy.begin(), heightsCopy.end());
    REQUIRE(heights.size() == 6);
    REQUIRE_FALSE(heights.empty());
    heights.insert(heightsCopy.begin(), heightsCopy.end());
    REQUIRE(heights.size() == 6);
    REQUIRE_FALSE(heights.empty());
    REQUIRE(heights == heightsCopy);

    heights.clear();
    REQUIRE(heights.size() == 0);
    REQUIRE(heights.empty());
    heights.swap(heightsCopy);
    REQUIRE(heights.size() == 6);
    REQUIRE_FALSE(heights.empty());
    REQUIRE(heightsCopy.size() == 0);
    REQUIRE(heightsCopy.empty());
    REQUIRE(heights != heightsCopy);

    REQUIRE(heights.find("Kozume")->first == "Kozume");
    REQUIRE(heights.count("Kozume") == 1);
    REQUIRE(heights.find("Tsukishima")->first == "Tsukishima");
    REQUIRE(heights.count("Tsukishima") == 1);
    REQUIRE(heights.find("Ayanami") == heights.end());
    REQUIRE(heights.count("Ayanami") == 0);

    //std::cout << "heights = " << heights << "\n";
    REQUIRE_THAT(heights, EqualsRange<decltype(heights)>(
        "{ [Azumane] -> 184.7, [Hinata] -> 162.8, [Kageyama] -> 180.6, [Kozume] -> 169.2, [Kuroo] -> 187.7, [Tsukishima] -> 188.3 }"
    ));

    auto eraseResult = heights.erase(heights.find("Kozume"));
    REQUIRE(eraseResult == heights.find("Kuroo"));
    REQUIRE(heights.size() == 5);
    REQUIRE_FALSE(heights.empty());
    //std::cout << "heights (erase Kozume) = " << heights << "\n";
    REQUIRE_THAT(heights, EqualsRange<decltype(heights)>(
        "{ [Azumane] -> 184.7, [Hinata] -> 162.8, [Kageyama] -> 180.6, [Kuroo] -> 187.7, [Tsukishima] -> 188.3 }"
    ));

    REQUIRE(heights.erase("Azumane") == 1);
    REQUIRE(heights.size() == 4);
    REQUIRE_FALSE(heights.empty());
    REQUIRE(heights.erase("unknown") == 0);
    REQUIRE(heights.size() == 4);
    REQUIRE_FALSE(heights.empty());
    //std::cout << "heights (erase Azumane) = " << heights << "\n";
    REQUIRE_THAT(heights, EqualsRange<decltype(heights)>(
        "{ [Hinata] -> 162.8, [Kageyama] -> 180.6, [Kuroo] -> 187.7, [Tsukishima] -> 188.3 }"
    ));

    eraseResult = heights.erase(heights.find("Kageyama"), heights.find("Tsukishima"));
    REQUIRE(eraseResult == heights.find("Tsukishima"));
    REQUIRE(heights.size() == 2);
    REQUIRE_FALSE(heights.empty());
    //std::cout << "heights (erase [Kageyama, Tsukishima) range)  = " << heights << "\n";
    REQUIRE_THAT(heights, EqualsRange<decltype(heights)>(
        "{ [Hinata] -> 162.8, [Tsukishima] -> 188.3 }"
    ));

    Map<std::string, std::string> letters;

    // Use pair's move constructor.
    letters.emplace(std::make_pair(std::string("a"), std::string("a")));

    // Use pair's converting move constructor.
    letters.emplace(std::make_pair("b", "abcd"));

    // Use pair's template constructor.
    letters.emplace("d", "ddd");

    // Use pair's piecewise constructor.
    letters.emplace(std::piecewise_construct, std::forward_as_tuple("c"), std::forward_as_tuple(10, 'c'));

    //std::cout << "letters = " << letters << "\n";
    REQUIRE_THAT(letters, EqualsRange<decltype(letters)>(
        "{ [a] -> a, [b] -> abcd, [c] -> cccccccccc, [d] -> ddd }"
    ));
}

TEST_CASE("Test modification", "[FlatMap]") {
    testModify<std::map>();
    testModify<FlatMap>();
}

int idCounter = 0;
template<typename T>
class CustomType {
public:
    CustomType() : t(), id(idCounter++) {
        std::cout << "CustomType<" << typeid(T).name() << ">::CustomType() id" << id << " default ctor\n";
    }
    CustomType(const T& t) : t(t), id(idCounter++) {
        std::cout << "CustomType<" << typeid(T).name() << ">::CustomType() id" << id << " lvalue ctor, t=" << this->t << "\n";
    }
    CustomType(T&& t) : t(std::move(t)), id(idCounter++) {
        std::cout << "CustomType<" << typeid(T).name() << ">::CustomType() id" << id << " rvalue ctor, t=" << this->t << "\n";
    }
    CustomType(const CustomType& other) : t(other.t), id(idCounter++) {
        std::cout << "CustomType<" << typeid(T).name() << ">::CustomType() id" << id << " copy ctor, t=" << this->t << "\n";
    }
    CustomType(CustomType&& other) noexcept : t(std::move(other.t)), id(idCounter++) {
        std::cout << "CustomType<" << typeid(T).name() << ">::CustomType() id" << id << " move ctor, t=" << this->t << "\n";
    }
    CustomType& operator=(const CustomType& other) {
        t = other.t;
        std::cout << "CustomType<" << typeid(T).name() << ">::operator=() id" << id << " copy assign, t=" << this->t << "\n";
        return *this;
    }
    CustomType& operator=(CustomType&& other) noexcept {
        t = std::move(other.t);
        std::cout << "CustomType<" << typeid(T).name() << ">::operator=() id" << id << " move assign, t=" << this->t << "\n";
        return *this;
    }
    ~CustomType() {
        std::cout << "CustomType<" << typeid(T).name() << ">::~CustomType() id" << id << " dtor, t=" << this->t << "\n";
    }
    const T& get() const {
        return t;
    }
    bool operator<(const CustomType& other) const {
        return t < other.t;
    }

private:
    T t;
    int id;

    friend std::ostream& operator<<(std::ostream& out, const CustomType& custom) {
        return out << "(id" << custom.id << ":" << custom.t << ")";
    }
};

template<template<typename...> class Map>
void testMoveSemantics() {
    std::cout << "\nMap type is " << typeid(Map<CustomType<std::string>, CustomType<int>>).name() << "\n";

    Map<CustomType<std::string>, CustomType<int>> colors;
    std::cout << "\nAssign color red from rvalues.\n";
    colors[CustomType<std::string>("red")] = 0xff0000;

    std::cout << "\nAssign color green from lvalues.\n";
    CustomType<std::string> greenName("green");
    CustomType<int> greenColor(0x00ff00);
    colors[greenName] = greenColor;

    std::cout << "\nConstruct copy of colors.\n";
    decltype(colors) colors2(colors.begin(), colors.end());

    std::cout << "\nInsert blue from lvalue.\n";
    typename decltype(colors2)::value_type blueValue("blue", 0x0000ff);
    colors2.insert(blueValue);

    std::cout << "\nInsert yellow from rvalue.\n";
    colors2.insert(typename decltype(colors2)::value_type("yellow", 0xffff00));

    std::cout << "\nEmplace white from rvalues.\n";
    colors2.emplace("white", 0xffffff);

    std::cout << "\ncolors2 = " << colors2 << "\n";
}

TEST_CASE("Test move semantics", "[.][FlatMap]") {
    testMoveSemantics<std::map>();
    testMoveSemantics<FlatMap>();
}

struct MyStruct {
    MyStruct() : data(), bits() {}
    MyStruct(int x) : data(), bits(x) { data.fill(x); }
    std::array<int, 24> data;
    std::bitset<64> bits;

    friend std::ostream& operator<<(std::ostream& out, const MyStruct& x) {
        return out << "MyStruct(" << x.data[0] << ", " << x.bits << ")";
    }
};

// Randomly chooses to allocate some data in heap, causing fragmentation for
// objects that don't use contiguous memory (like std::map).
void randomHeapAllocate(double randVal, std::vector<int*>& numberArray) {
    if (randVal < 0.75) {
        numberArray.push_back(new int[64]);
    }
}

template<template<typename...> class Map>
void testPerformance() {
    std::cout << "\nMap type is " << typeid(Map<CustomType<std::string>, CustomType<int>>).name() << "\n";

    std::mt19937 mersenneRand(123);
    std::uniform_real_distribution<> dist1(0.0, 1.0);
    std::uniform_int_distribution<> distN(1, 50000);
    std::vector<int*> numberArray;

    // Emplace single elements randomly.
    auto t1 = std::chrono::high_resolution_clock::now();
    Map<int, MyStruct> map1;
    int map1EmplaceCount = 0;
    for (int i = 0; i < 50000; ++i) {
        auto result = map1.emplace(distN(mersenneRand), distN(mersenneRand));
        map1EmplaceCount += result.second;

        randomHeapAllocate(dist1(mersenneRand), numberArray);
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    // Iteration.
    long long map1EmplaceSum = 0;
    for (const auto& v : map1) {
        map1EmplaceSum += v.second.data[0];
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    // Random search.
    int map1SearchCount = 0;
    long long map1SearchSum = 0;
    for (int i = 0; i < 1000; ++i) {
        auto result = map1.find(distN(mersenneRand));
        if (result != map1.end()) {
            ++map1SearchCount;
            map1SearchSum += result->second.data[0];
        }
    }
    auto t4 = std::chrono::high_resolution_clock::now();

    // Insert range of random elements.
    Map<double, MyStruct> map2;
    for (int i = 0; i < 1000; ++i) {
        map2.insert({
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
            typename decltype(map2)::value_type(dist1(mersenneRand), distN(mersenneRand)),
        });
    }
    auto t5 = std::chrono::high_resolution_clock::now();

    // Iteration part 2.
    long long map2InsertSum = 0;
    for (const auto& v : map2) {
        map2InsertSum += v.second.data[0];
    }
    auto t6 = std::chrono::high_resolution_clock::now();

    std::cout << "Emplace time: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << "us\n";
    std::cout << "map1EmplaceCount = " << map1EmplaceCount << ", numberArray.size() = " << numberArray.size() << "\n\n";

    std::cout << "Iteration time: " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() << "us\n";
    std::cout << "map1EmplaceSum = " << map1EmplaceSum << "\n\n";

    std::cout << "Search time: " << std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count() << "us\n";
    std::cout << "map1SearchCount = " << map1SearchCount << ", map1SearchSum = " << map1SearchSum << "\n\n";

    std::cout << "Insert range time: " << std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count() << "us\n";
    std::cout << "map2.size() = " << map2.size() << "\n\n";

    std::cout << "Iteration time: " << std::chrono::duration_cast<std::chrono::microseconds>(t6 - t5).count() << "us\n";
    std::cout << "map2InsertSum = " << map2InsertSum << "\n\n";

    for (auto& x : numberArray) {
        delete[] x;
    }
}

TEST_CASE("Test performance", "[!benchmark][FlatMap]") {
    testPerformance<std::map>();
    testPerformance<FlatMap>();
}

/*template<typename T>
struct CustomComp {
    CustomComp() {
        std::cout << "CustomComp::CustomComp()\n";
    }
    inline bool operator()(const T& lhs, const T& rhs) {
        return lhs > rhs;
    }
};

int main() {
    std::cout << "Started FlatMap test.\n";

    std::map<int, std::string> stuff;
    stuff.key_comp();
    stuff.emplace(1, "test");

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

    std::cout << "m = " << m << "\n";
    std::cout << "flat = " << flat << "\n";

    std::cout << "\n\n======== testCtor<std::map>() ========\n";
    testCtor<std::map>();
    std::cout << "\n======== testCtor<FlatMap>() ========\n";
    testCtor<FlatMap>();

    std::cout << "\n\n======== testAccess<std::map>() ========\n";
    testAccess<std::map>();
    std::cout << "\n======== testAccess<FlatMap>() ========\n";
    testAccess<FlatMap>();

    std::cout << "\n\n======== testIter<std::map>() ========\n";
    testIter<std::map>();
    std::cout << "\n======== testIter<FlatMap>() ========\n";
    testIter<FlatMap>();

    std::cout << "\n\n======== testModify<std::map>() ========\n";
    testModify<std::map>();
    std::cout << "\n======== testModify<FlatMap>() ========\n";
    testModify<FlatMap>();

    std::cout << "\n\n======== testMoveSemantics<std::map>() ========\n";
    testMoveSemantics<std::map>();
    std::cout << "\n======== testMoveSemantics<FlatMap>() ========\n";
    testMoveSemantics<FlatMap>();

    std::cout << "\n\n======== testPerformance<std::map>() ========\n";
    testPerformance<std::map>();
    std::cout << "\n======== testPerformance<FlatMap>() ========\n";
    testPerformance<FlatMap>();

    return 0;
}*/
