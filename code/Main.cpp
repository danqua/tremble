#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

struct Sector;

struct Wall
{
    glm::ivec2 v1;
    glm::ivec2 v2;
    int front;
    int back;

    void Draw(SDL_Renderer* renderer)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderLine(renderer, v1[0], v1[1], v2[0], v2[1]);
    }

    void DrawNormal(SDL_Renderer* renderer) const
    {
        glm::vec2 normal = GetNormal();
        int x1 = v1[0] + (v2[0] - v1[0]) / 2;
        int y1 = v1[1] + (v2[1] - v1[1]) / 2;
        int x2 = x1 + static_cast<int>(normal.x * 4.0f);
        int y2 = y1 + static_cast<int>(normal.y * 4.0f);

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderLine(renderer, x1, y1, x2, y2);
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
    int numWalls;
    Wall* walls;
};

static void DrawGrid(SDL_Renderer* renderer, int cellSize, SDL_Color color)
{
    int width;
    int height;
    SDL_GetRenderLogicalPresentation(renderer, &width, &height, NULL, NULL);

    int cols = width / cellSize;
    int rows = height / cellSize;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int i = 0; i < rows; ++i)
    {
        SDL_RenderLine(renderer, 0, i * cellSize, width, i * cellSize);
    }

    for (int i = 0; i < cols; ++i)
    {
        SDL_RenderLine(renderer, i * cellSize, 0, i * cellSize, height);
    }
}

static bool PointInSector(const Sector& sector, const glm::vec2& point)
{
    for (int i = 0; i < sector.numWalls; ++i)
    {
        const Wall& wall = sector.walls[i];
        glm::vec2 a = glm::vec2(wall.v1[0], wall.v1[1]);
        glm::vec2 b = glm::vec2(wall.v2[0], wall.v2[1]);
        glm::vec2 normal = glm::normalize(b - a);
        glm::vec2 pointToA = point - a;
        float dot = glm::dot(normal, pointToA);
        if (dot < 0.0f)
        {
            return false;
        }
    }
    return true;
}

#include "Math/Line2.h"


int main(int argc, char** argv)
{
    int windowWidth  = 960;
    int windowHeight = 720;

    SDL_Window* window = SDL_CreateWindow("Tremble", windowWidth, windowHeight, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (!gladLoadGL())
    {
        return -1;
    }

    bool running = true;

    int mx = 0;
    int my = 0;



    while (running)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            else if (ev.type == SDL_EVENT_KEY_DOWN)
            {
                if (ev.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
            else if (ev.type == SDL_EVENT_MOUSE_MOTION)
            {
                mx = ev.motion.x;
                my = ev.motion.y;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();



        SDL_GL_SwapWindow(window);
    }

    return 0;
}
