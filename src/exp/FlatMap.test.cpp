#include <FlatMap.h>

#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <random>
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

    Map<std::string, int> range3(map1.begin(), map1.begin());
    std::cout << "range ctor (empty) = " << range3 << "\n";

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

template<template<typename...> class Map>
void testIter() {
    Map<int, std::string> map1 {
        {101, "emet"},
        {56, "dolor"},
        {-4, "lorem"},
        {79, "sit"},
        {3, "ipsum"}
    };
    std::cout << "map1 forward: { ";
    for (auto it = map1.cbegin(); it != map1.cend(); ++it) {
        std::cout << "[" << it->first << "] -> " << it->second << ", ";
    }
    std::cout << "}\n";
    std::cout << "map1 reverse: { ";
    for (auto it = map1.rbegin(); it != map1.rend(); ++it) {
        std::cout << "[" << it->first << "] -> " << it->second << ", ";
    }
    std::cout << "}\n";

    const Map<int, std::string, std::greater<int>> map2 {
        {101, "emet"},
        {56, "dolor"},
        {-4, "lorem"},
        {79, "sit"},
        {3, "ipsum"}
    };
    std::cout << "map2 forward: { ";
    for (auto it = map2.begin(); it != map2.end(); ++it) {
        std::cout << "[" << it->first << "] -> " << it->second << ", ";
    }
    std::cout << "}\n";
    std::cout << "map2 reverse: { ";
    for (auto it = map2.crbegin(); it != map2.crend(); ++it) {
        std::cout << "[" << it->first << "] -> " << it->second << ", ";
    }
    std::cout << "}\n";

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
    std::cout << "magnitudes = { ";
    for (auto iter = magnitudes.begin(); iter != magnitudes.end(); ++iter) {
        auto p = iter->first;
        magnitudes[p] = std::hypot(p->x, p->y);
        std::cout << "[(" << p->x << ", " << p->y << ")] -> " << iter->second << ", ";
    }
    std::cout << "}\n";

    // Repeat the above with the range-based for loop.
    std::cout << "magnitudes = { ";
    for (auto i : magnitudes) {
        auto p = i.first;
        p->y = i.second;
        magnitudes[p] = std::hypot(p->x, p->y);
        std::cout << "[(" << p->x << ", " << p->y << ")] -> " << magnitudes[p] << ", ";
    }
    std::cout << "}\n";
}

template<typename Iter, typename Key, typename Value>
void checkInsert(const std::pair<Iter, bool>& result, const std::pair<Key, Value>& expectedNode, bool expectedStatus) {
    //std::cout << "check {{" << result.first->first << ", " << result.first->second << "}, " << result.second << "}\n";
    //std::cout << "matches {{" << expectedNode.first << ", " << expectedNode.second << "}, " << expectedStatus << "}\n";
    assert(result.first->first == expectedNode.first);
    assert(result.first->second == expectedNode.second);
    assert(result.second == expectedStatus);
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
void testModify() {
    Map<std::string, float> heights;
    assert(heights.size() == 0 && heights.empty());

    // Insert from rvalue reference.
    const auto insertResult = heights.insert({"Hinata", 162.8f});
    checkInsert(insertResult, std::pair<std::string, float>("Hinata", 162.8f), true);
    assert(heights.size() == 1 && !heights.empty());
    assert(heights.find("Hinata") == insertResult.first);
    assert(heights.find("Kageyama") == heights.end());

    // Insert from lvalue reference.
    const auto insertResult2 = heights.insert(*insertResult.first);
    checkInsert(insertResult2, *insertResult.first, false);
    assert(heights.size() == 1 && !heights.empty());

    std::pair<std::string, float> nodeAzumane("Azumane", 184.7f);
    const auto insertResult3 = heights.insert(nodeAzumane);
    checkInsert(insertResult3, nodeAzumane, true);
    assert(heights.size() == 2 && !heights.empty());

    // Insert via forwarding to emplace.
    const auto insertResult4 = heights.insert(std::pair<std::string, float>{"Kageyama", 180.6f});
    checkInsert(insertResult4, std::pair<std::string, float>("Kageyama", 180.6f), true);
    assert(heights.size() == 3 && !heights.empty());

    // Insert from iterator range.
    Map<std::string, float> heightsCopy;
    heightsCopy.insert(std::begin(heights), std::begin(heights));
    assert(heightsCopy.size() == 0 && heightsCopy.empty());
    heightsCopy.insert(std::begin(heights), std::end(heights));
    assert(heightsCopy.size() == 3 && !heightsCopy.empty());

    // Insert from initializer_list.
    heightsCopy.insert({{"Kozume", 169.2f}, {"Kuroo", 187.7f}});
    assert(heightsCopy.size() == 5 && !heightsCopy.empty());

    heightsCopy.insert({{"Tsukishima", 188.3f}, {"Kageyama", -99.8f}});
    assert(heightsCopy.size() == 6 && !heightsCopy.empty());

    heightsCopy.insert({{"Azumane", 0.0f}, {"Azumane", 0.0f}, {"Azumane", 0.0f}});
    assert(heightsCopy.size() == 6 && !heightsCopy.empty());

    // Insert partial duplicates, and a full range.
    heights.insert(heightsCopy.begin(), heightsCopy.end());
    assert(heights.size() == 6 && !heights.empty());
    heights.clear();
    assert(heights.size() == 0 && heights.empty());
    heights.insert(heightsCopy.begin(), heightsCopy.end());
    assert(heights.size() == 6 && !heights.empty());
    heights.insert(heightsCopy.begin(), heightsCopy.end());
    assert(heights.size() == 6 && !heights.empty());
    assert(heights == heightsCopy);

    heights.clear();
    assert(heights.size() == 0 && heights.empty());
    heights.swap(heightsCopy);
    assert(heights.size() == 6 && !heights.empty());
    assert(heightsCopy.size() == 0 && heightsCopy.empty());
    assert(heights != heightsCopy);

    assert(heights.find("Kozume")->first == "Kozume");
    assert(heights.count("Kozume") == 1);
    assert(heights.find("Tsukishima")->first == "Tsukishima");
    assert(heights.count("Tsukishima") == 1);
    assert(heights.find("Ayanami") == heights.end());
    assert(heights.count("Ayanami") == 0);

    std::cout << "heights = " << heights << "\n";

    auto eraseResult = heights.erase(heights.find("Kozume"));
    assert(eraseResult == heights.find("Kuroo"));
    assert(heights.size() == 5 && !heights.empty());
    std::cout << "heights (erase Kozume) = " << heights << "\n";

    assert(heights.erase("Azumane") == 1);
    assert(heights.size() == 4 && !heights.empty());
    assert(heights.erase("unknown") == 0);
    assert(heights.size() == 4 && !heights.empty());
    std::cout << "heights (erase Azumane) = " << heights << "\n";

    eraseResult = heights.erase(heights.find("Kageyama"), heights.find("Tsukishima"));
    assert(eraseResult == heights.find("Tsukishima"));
    assert(heights.size() == 2 && !heights.empty());
    std::cout << "heights (erase [Kageyama, Tsukishima) range)  = " << heights << "\n";

    Map<std::string, std::string> letters;

    // Use pair's move constructor.
    letters.emplace(std::make_pair(std::string("a"), std::string("a")));

    // Use pair's converting move constructor.
    letters.emplace(std::make_pair("b", "abcd"));

    // Use pair's template constructor.
    letters.emplace("d", "ddd");

    // Use pair's piecewise constructor.
    letters.emplace(std::piecewise_construct, std::forward_as_tuple("c"), std::forward_as_tuple(10, 'c'));

    std::cout << "letters = " << letters << "\n";
}

template<template<typename...> class Map>
void testMoveSemantics() {
    Map<CustomType<std::string>, CustomType<int>> colors;
    std::cout << "Assign color red from rvalues.\n";
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

int main() {
    std::cout << "Started FlatMap test.\n";

    std::map<int, std::string> stuff;
    stuff.key_comp();
    stuff.emplace(1, "test");

    /*
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

    /*std::cout << "\n\n======== testCtor<std::map>() ========\n";
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
    testModify<FlatMap>();*/

    /*std::cout << "\n\n======== testMoveSemantics<std::map>() ========\n";
    testMoveSemantics<std::map>();
    std::cout << "\n======== testMoveSemantics<FlatMap>() ========\n";
    testMoveSemantics<FlatMap>();*/

    std::cout << "\n\n======== testPerformance<std::map>() ========\n";
    testPerformance<std::map>();
    std::cout << "\n======== testPerformance<FlatMap>() ========\n";
    testPerformance<FlatMap>();

    return 0;
}
