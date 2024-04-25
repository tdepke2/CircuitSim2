#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>

/**
 * Reference RGB to HSV conversions used for testing copied from Dear ImGui sources
 * 
 * See: https://github.com/ocornut/imgui
 */
namespace ImGui {

#define ImFabs(X)           fabsf(X)
#define ImFmod(X, Y)        fmodf((X), (Y))

template<typename T> static inline void ImSwap(T& a, T& b)    { T tmp = a; a = b; b = tmp; }

// Convert rgb floats ([0-1],[0-1],[0-1]) to hsv floats ([0-1],[0-1],[0-1]), from Foley & van Dam p592
// Optimized http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
void ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
    float K = 0.f;
    if (g < b)
    {
        ImSwap(g, b);
        K = -1.f;
    }
    if (r < g)
    {
        ImSwap(r, g);
        K = -2.f / 6.f - K;
    }

    const float chroma = r - (g < b ? g : b);
    out_h = ImFabs(K + (g - b) / (6.f * chroma + 1e-20f));
    out_s = chroma / (r + 1e-20f);
    out_v = r;
}

// Convert hsv floats ([0-1],[0-1],[0-1]) to rgb floats ([0-1],[0-1],[0-1]), from Foley & van Dam p593
// also http://en.wikipedia.org/wiki/HSL_and_HSV
void ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
    if (s == 0.0f)
    {
        // gray
        out_r = out_g = out_b = v;
        return;
    }

    h = ImFmod(h, 1.0f) / (60.0f / 360.0f);
    int   i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0: out_r = v; out_g = t; out_b = p; break;
    case 1: out_r = q; out_g = v; out_b = p; break;
    case 2: out_r = p; out_g = v; out_b = t; break;
    case 3: out_r = p; out_g = q; out_b = v; break;
    case 4: out_r = t; out_g = p; out_b = v; break;
    case 5: default: out_r = v; out_g = p; out_b = q; break;
    }
}

}

// Based on information found on the following page:
// https://en.wikipedia.org/wiki/HSL_and_HSV
sf::Vector3f convertRgbToHsv(const sf::Vector3f& rgb) {
    float value = std::max(std::max(rgb.x, rgb.y), rgb.z);
    float chroma = value - std::min(std::min(rgb.x, rgb.y), rgb.z);
    float hue;
    if (chroma == 0.0f) {
        hue = 0.0f;
    } else if (value == rgb.x) {
        hue = (rgb.y - rgb.z) / chroma + 6.0f;
    } else if (value == rgb.y) {
        hue = (rgb.z - rgb.x) / chroma + 2.0f;
    } else {    // value == rgb.z
        hue = (rgb.x - rgb.y) / chroma + 4.0f;
    }
    hue = std::fmod(hue, 6.0f) / 6.0f;

    return {
        hue, (value == 0.0f ? 0.0f : chroma / value), value
    };
}

sf::Vector3f convertHsvToRgb(const sf::Vector3f& hsv) {
    float chroma = hsv.y * hsv.z;
    float m = hsv.z - chroma;
    float x = chroma * (1.0f - std::fabs(std::fmod(hsv.x * 6.0f, 2.0f) - 1.0f));

    int hueSection = static_cast<int>(hsv.x * 6.0f);
    switch(hueSection) {
    case 0:
        return {hsv.z, x + m, m};
    case 1:
        return {x + m, hsv.z, m};
    case 2:
        return {m, hsv.z, x + m};
    case 3:
        return {m, x + m, hsv.z};
    case 4:
        return {x + m, m, hsv.z};
    default:
        return {hsv.z, m, x + m};
    }
}

bool floatEqual(float a, float b) {
    return std::fabs(a - b) < 1e-6f;
}

float findTotalError(const sf::Vector3f& a, const sf::Vector3f& b) {
    return std::fabs(a.x - b.x) + std::fabs(a.y - b.y) + std::fabs(a.z - b.z);
}

void testRgbToHsv(const sf::Vector3f& rgb) {
    sf::Vector3f hsv;
    ImGui::ColorConvertRGBtoHSV(rgb.x, rgb.y, rgb.z, hsv.x, hsv.y, hsv.z);
    sf::Vector3f rgb2;
    ImGui::ColorConvertHSVtoRGB(hsv.x, hsv.y, hsv.z, rgb2.x, rgb2.y, rgb2.z);

    sf::Vector3f myHsv = convertRgbToHsv(rgb);
    sf::Vector3f myRgb2 = convertHsvToRgb(myHsv);

    float err = findTotalError(rgb, rgb2);
    float myErr = findTotalError(rgb, myRgb2);

    std::cout << std::setprecision(10);
    std::cout << "rgb(" << std::setw(13) << rgb.x << ", " << std::setw(13) << rgb.y << ", " << std::setw(13) << rgb.z
        << ") -> hsv(" << std::setw(13) << hsv.x << ", " << std::setw(13) << hsv.y << ", " << std::setw(13) << hsv.z
        << "), err = " << std::setw(13) << err << ", myErr = " << std::setw(13) << myErr << "\n";

    if (floatEqual(hsv.x, myHsv.x) && floatEqual(hsv.y, myHsv.y) && floatEqual(hsv.z, myHsv.z)) {
    } else {
        std::cout << std::setw(56) << "MISMATCH: got myHsv(" << std::setw(13) << myHsv.x << ", " << std::setw(13) << myHsv.y << ", " << std::setw(13) << myHsv.z << ")\n";
        assert(false);
    }
    assert(floatEqual(err, 0.0f));
    assert(floatEqual(myErr, 0.0f));
}

void testHsvToRgb(const sf::Vector3f& hsv) {
    sf::Vector3f rgb;
    ImGui::ColorConvertHSVtoRGB(hsv.x, hsv.y, hsv.z, rgb.x, rgb.y, rgb.z);
    sf::Vector3f hsv2;
    ImGui::ColorConvertRGBtoHSV(rgb.x, rgb.y, rgb.z, hsv2.x, hsv2.y, hsv2.z);

    sf::Vector3f myRgb = convertHsvToRgb(hsv);
    sf::Vector3f myHsv2 = convertRgbToHsv(myRgb);

    float err = findTotalError(hsv, hsv2);
    float myErr = findTotalError(hsv, myHsv2);

    std::cout << std::setprecision(10);
    std::cout << "hsv(" << std::setw(13) << hsv.x << ", " << std::setw(13) << hsv.y << ", " << std::setw(13) << hsv.z
        << ") -> rgb(" << std::setw(13) << rgb.x << ", " << std::setw(13) << rgb.y << ", " << std::setw(13) << rgb.z
        << "), err = " << std::setw(13) << err << ", myErr = " << std::setw(13) << myErr << "\n";

    if (floatEqual(rgb.x, myRgb.x) && floatEqual(rgb.y, myRgb.y) && floatEqual(rgb.z, myRgb.z)) {
    } else {
        std::cout << std::setw(56) << "MISMATCH: got myRgb(" << std::setw(13) << myRgb.x << ", " << std::setw(13) << myRgb.y << ", " << std::setw(13) << myRgb.z << ")\n";
        assert(false);
    }
    if (hsv.x != 1.0f && hsv.y != 0.0f && hsv.z != 0.0f) {
        //assert(floatEqual(err, 0.0f));
        assert(floatEqual(myErr, 0.0f));
    }
}

int main() {
    constexpr int states = 6, states2 = states * states, states3 = states2 * states;
    std::cout << "\ntestRgbToHsv()\n";
    for (int i = 0; i < states3; ++i) {
        testRgbToHsv({
            static_cast<float>(i / states2 % states) / (states - 1),
            static_cast<float>(i / states % states) / (states - 1),
            static_cast<float>(i % states) / (states - 1)
        });
    }
    std::cout << "\ntestHsvToRgb()\n";
    for (int i = 0; i < states3; ++i) {
        testHsvToRgb({
            static_cast<float>(i / states2 % states) / (states - 1),
            static_cast<float>(i / states % states) / (states - 1),
            static_cast<float>(i % states) / (states - 1)
        });
    }
    std::cout << "\ntest random samples\n";
    std::mt19937 mersenneRand;
    mersenneRand.seed(123);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int i = 0; i < 100; ++i) {
        testRgbToHsv({dist(mersenneRand), dist(mersenneRand), dist(mersenneRand)});
        testHsvToRgb({dist(mersenneRand), dist(mersenneRand), dist(mersenneRand)});
    }

    return 0;
}
