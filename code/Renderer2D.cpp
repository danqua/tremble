#include "Renderer2D.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

Color Color::White     = Color(255, 255, 255);
Color Color::Black     = Color(  0,   0,   0);
Color Color::Red       = Color(255,   0,   0);
Color Color::Green     = Color(  0, 255,   0);
Color Color::Blue      = Color(  0,   0, 255);
Color Color::Yellow    = Color(255, 255,   0);
Color Color::Magenta   = Color(255,   0, 255);
Color Color::Cyan      = Color(  0, 255, 255);
Color Color::Gray      = Color(127, 127, 127);
Color Color::LightGray = Color(191, 191, 191);
Color Color::DarkGray  = Color( 63,  63,  63);

void Renderer2D::Init(GLFWwindow* window)
{
    lineCount = 0;
    rectCount = 0;
    drawColor = Color(0, 0, 0, 255);

    this->window = window;
    glfwGetWindowSize(window, &width, &height);
    SetSize(width, height);
}

void Renderer2D::Shutdown()
{
}

void Renderer2D::SetSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

void Renderer2D::Clear()
{
    float r = drawColor.r / 255.0f;
    float g = drawColor.g / 255.0f;
    float b = drawColor.b / 255.0f;
    float a = drawColor.a / 255.0f;

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
}

void Renderer2D::SetDrawColor(Color color)
{
    drawColor = color;
}

void Renderer2D::DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (lineCount >= kMaxLines)
    {
        Flush();
    }
    
    LineData& line = lines[lineCount++];
    line.x1 = x1;
    line.y1 = y1;
    line.x2 = x2;
    line.y2 = y2;
    line.color = drawColor;
}

void Renderer2D::DrawRect(int32_t x, int32_t y, int32_t w, int32_t h)
{
    DrawLine(x    , y    , x + w, y    );
    DrawLine(x + w, y    , x + w, y + h);
    DrawLine(x + w, y + h, x    , y + h);
    DrawLine(x    , y + h, x    , y    );
}

void Renderer2D::FillRect(int32_t x, int32_t y, int32_t w, int32_t h)
{
    if (rectCount >= kMaxRects)
    {
        Flush();
    }

    RectData& rect = rects[rectCount++];
    rect.x = x;
    rect.y = y;
    rect.width = w;
    rect.height = h;
    rect.color = drawColor;
}

void Renderer2D::Present()
{
    Flush();
    glfwSwapBuffers(window);
}

void Renderer2D::Flush()
{
    // Draw lines
    glBegin(GL_LINES);

    for (int i = 0; i < lineCount; ++i)
    {
        LineData& line = lines[i];
        glColor4ub(line.color.r, line.color.g, line.color.b, line.color.a);
        glVertex2i(line.x1, line.y1);
        glVertex2i(line.x2, line.y2);
    }

    glEnd();

    lineCount = 0;

    // Draw rects
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < rectCount; ++i)
    {
        RectData& rect = rects[i];

        int x1 = rect.x;
        int y1 = rect.y;
        int x2 = rect.x + rect.width;
        int y2 = rect.y + rect.height;

        glColor4ub(rect.color.r, rect.color.g, rect.color.b, rect.color.a);
        glVertex2i(x1, y1);
        glVertex2i(x2, y1);
        glVertex2i(x2, y2);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glVertex2i(x1, y2);
    }
    glEnd();

    rectCount = 0;
}
