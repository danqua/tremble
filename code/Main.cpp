#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

#include "Renderer2D.h"

struct WindowData
{
    int mouse[2];
};

struct Sector;

struct Wall
{
    int v1[2];
    int v2[2];
    int front;
    int back;

    void Draw(Renderer2D& renderer)
    {
        renderer.SetDrawColor(Color::White);
        renderer.DrawLine(v1[0], v1[1], v2[0], v2[1]);
    }

    void DrawNormal(Renderer2D& renderer) const
    {
        glm::vec2 normal = GetNormal();
        int x1 = v1[0] + (v2[0] - v1[0]) / 2;
        int y1 = v1[1] + (v2[1] - v1[1]) / 2;
        int x2 = x1 + static_cast<int>(normal.x * 4.0f);
        int y2 = y1 + static_cast<int>(normal.y * 4.0f);

        renderer.SetDrawColor(Color::Yellow);
        renderer.DrawLine(x1, y1, x2, y2);
    }

    glm::vec2 GetNormal() const
    {
        glm::vec2 a = glm::vec2(v1[0], v1[1]);
        glm::vec2 b = glm::vec2(v2[0], v2[1]);
        glm::vec2 normal = glm::normalize(b - a);
        return glm::vec2(-normal.y, normal.x);
    }
};

struct Sector
{
    
};

static void DrawGrid(Renderer2D& renderer, int cellSize, Color color)
{
    int width, height;
    glfwGetWindowSize(renderer.GetWindow(), &width, &height);

    int cols = width / cellSize;
    int rows = height / cellSize;

    renderer.SetDrawColor(color);

    for (int i = 0; i < rows; ++i)
    {
        renderer.DrawLine(0, i * cellSize, width, i * cellSize);
    }

    for (int i = 0; i < cols; ++i)
    {
        renderer.DrawLine(i * cellSize, 0, i * cellSize, height);
    }
}

int main(int argc, char** argv)
{
    if (!glfwInit())
    {
        return -1;
    }

    int windowWidth  = 640;
    int windowHeight = 480;

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Hello World", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwSetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwSetWindowAttrib(window, GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    WindowData windowData = {};

    glfwSetWindowUserPointer(window, &windowData);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        WindowData* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data->mouse[0] = static_cast<int>(xpos);
        data->mouse[1] = static_cast<int>(ypos);
    });

    Renderer2D& renderer = Renderer2D::Get();
    renderer.Init(window);
    renderer.SetSize(windowWidth / 2, windowHeight / 2);

    int numWalls = 8;
    Wall walls[] = {
        { {   0,  0 }, { 160,   0 },  0, -1 },
        { { 160,  0 }, { 160,  96 },  0, -1 },
        { { 160, 96 }, {   0,  96 },  0, -1 },
        { {   0, 96 }, {   0,   0 },  0, -1 },

        { {  64, 32 }, {  64,  64 },  0,  1 },
        { {  64, 64 }, {  96,  64 },  0,  1 },
        { {  96, 64 }, {  96,  32 },  0,  1 },
        { {  96, 32 }, {  64,  32 },  0,  1 }
    };

    
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        renderer.SetDrawColor(Color::Black);
        renderer.Clear();

        DrawGrid(renderer, 32, Color::DarkGray);

        for (int i = 0; i < numWalls; ++i)
        {
            walls[i].Draw(renderer);
            walls[i].DrawNormal(renderer);
        }


        renderer.Present();

        glfwPollEvents();
    }

    renderer.Shutdown();
    glfwTerminate();
    return 0;
}
