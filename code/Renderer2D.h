#pragma once
#include <stdint.h>
#include <array>

constexpr int kMaxLines = 1024;
constexpr int kMaxRects = 1024;

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

    static Color White;
    static Color Black;
    static Color Red;
    static Color Green;
    static Color Blue;
    static Color Yellow;
    static Color Magenta;
    static Color Cyan;
    static Color Gray;
    static Color LightGray;
    static Color DarkGray;
};

struct GLFWwindow;

class Renderer2D
{
    struct LineData
    {
        int32_t x1;
        int32_t y1;
        int32_t x2;
        int32_t y2;
        Color   color;
    };

    struct RectData
    {
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
        Color   color;
    };

public:
    static Renderer2D& Get()
    {
        static Renderer2D instance;
        return instance;
    }

    void Init(GLFWwindow* window);
    void Shutdown();
    void SetSize(int width, int height);
    void Clear();
    void SetDrawColor(Color color);
    void DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h);
    void FillRect(int32_t x, int32_t y, int32_t w, int32_t h);
    void Present();

    inline int GetWidth() const { return width; }
    inline int GetHeight() const { return height; }
    GLFWwindow* GetWindow() const { return window; }

private:
    void Flush();

private:
    int width;
    int height;
    int lineCount;
    int rectCount;
    Color drawColor;
    GLFWwindow* window;
    std::array<LineData, kMaxLines> lines;
    std::array<RectData, kMaxRects> rects;
};