#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/quaternion.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#include <algorithm>
#include <vector>
#include <functional>

#include "Math/Box.h"
#include "Math/Ray.h"
#include "Math/Frustum.h"
#include "Math/Intersection.h"

constexpr glm::vec3 kWorldUp      = glm::vec3(0.0f,  1.0f,  0.0f);
constexpr glm::vec3 kWorldForward = glm::vec3(0.0f,  0.0f, -1.0f);
constexpr glm::vec3 kWorldRight   = glm::vec3(1.0f,  0.0f,  0.0f);

glm::vec2 mouseDelta;

struct Camera
{
    glm::vec3 position;

    float fov;
    float aspect;
    float near;
    float far;
    float pitch;
    float yaw;
    float roll;

    void LookAt(const glm::vec3& target)
    {
        glm::vec3 direction = glm::normalize(target - position);
        pitch = glm::degrees(glm::asin(direction.y));
        yaw = glm::degrees(glm::atan(direction.z, direction.x)) + 90.0f;
        int y = 0;
    }
};

struct Movement
{
    float speed;
    float friction;
    float mouseSensitivity;
    glm::vec3 velocity;
};

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

struct Quad
{
    glm::vec3 center;
    glm::vec3 normal;
    float width;
    float height;
};

struct Face
{
    glm::vec3 uAxis;
    glm::vec3 vAxis;
    glm::vec3 normal;
    std::vector<glm::vec3> vertices;
};

struct Image
{
    int       width;
    int       height;
    uint32_t* pixels;
};

struct Texture
{
    GLuint id;
    int    width;
    int    height;
};

glm::vec3 GetForwardVector(const glm::vec3& rotation)
{
    return glm::normalize(glm::quat(glm::radians(rotation)) * kWorldForward);
}

glm::vec3 GetRightVector(const glm::vec3& rotation)
{
    return glm::normalize(glm::quat(glm::radians(rotation)) * kWorldRight);
}

glm::vec3 GetUpVector(const glm::vec3& rotation)
{
    return glm::normalize(glm::quat(glm::radians(rotation)) * kWorldUp);
}

glm::mat4 GetProjection(const Camera& camera)
{
    return glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
}

glm::vec3 GetCameraRotation(const Camera& camera)
{
    return glm::vec3(camera.pitch, camera.yaw, camera.roll);
}

glm::mat4 GetView(const Camera& camera)
{
    glm::vec3 forward = GetForwardVector(GetCameraRotation(camera));
    return glm::lookAt(camera.position, camera.position + forward, kWorldUp);
}

void GetQuadVertices(const Quad& quad, glm::vec3 vertices[4])
{
    glm::vec3 up;
    glm::vec3 right;

    if (glm::abs(glm::dot(quad.normal, kWorldUp)) > 0.99f)
    {
        up = glm::cross(quad.normal, kWorldRight);
        right = glm::cross(up, quad.normal);
    }
    else if (glm::abs(glm::dot(quad.normal, kWorldRight)) > 0.99f)
    {
        right = glm::cross(quad.normal, kWorldUp);
        up = glm::cross(quad.normal, right);
    }
    else
    {
        up = glm::cross(quad.normal, kWorldRight);
        right = glm::cross(up, quad.normal);
    }

    glm::vec3 halfR = right * quad.width * 0.5f;
    glm::vec3 halfU = up * quad.height * 0.5f;

    vertices[0] = quad.center - halfR - halfU;
    vertices[1] = quad.center + halfR - halfU;
    vertices[2] = quad.center + halfR + halfU;
    vertices[3] = quad.center - halfR + halfU;
}

void DrawPoint(const glm::vec3& point, const glm::vec3& color = glm::vec3(1.0f), float size = 1.0f)
{
    float currentPointSize = 0.0f;
    glGetFloatv(GL_POINT_SIZE, &currentPointSize);

    glPointSize(size);
    glBegin(GL_POINTS);
    glColor3f(color.r, color.g, color.b);
    glVertex3f(point.x, point.y, point.z);
    glEnd();

    glPointSize(currentPointSize);
}

void DrawPoints(const glm::vec3* points, size_t count, const glm::vec3& color = glm::vec3(1.0f), float size = 1.0f)
{
    float currentPointSize = 0.0f;
    glGetFloatv(GL_POINT_SIZE, &currentPointSize);

    glPointSize(size);
    glBegin(GL_POINTS);
    glColor3f(color.r, color.g, color.b);
    for (size_t i = 0; i < count; i++)
    {
        glVertex3f(points[i].x, points[i].y, points[i].z);
    }
    glEnd();

    glPointSize(currentPointSize);
}

void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color = glm::vec3(1.0f), float width = 1.0f)
{
    float currentLineWidth = 0.0f;
    glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);

    glLineWidth(width);
    glBegin(GL_LINES);
    glColor3f(color.r, color.g, color.b);
    glVertex3f(start.x, start.y, start.z);
    glVertex3f(end.x, end.y, end.z);
    glEnd();

    glLineWidth(currentLineWidth);
}

void DrawQuad(const Quad& quad, const glm::vec3& color = glm::vec3(1.0f))
{
    glm::vec3 v[4];
    GetQuadVertices(quad, v);

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(color.r, color.g, color.b);
    glVertex3f(v[0].x, v[0].y, v[0].z);
    glVertex3f(v[1].x, v[1].y, v[1].z);
    glVertex3f(v[2].x, v[2].y, v[2].z);
    glVertex3f(v[3].x, v[3].y, v[3].z);
    glEnd();
}

void DrawQuadLines(const Quad& quad, const glm::vec3& color = glm::vec3(1.0f))
{
    glm::vec3 v[4];
    GetQuadVertices(quad, v);

    glBegin(GL_LINE_LOOP);
    glColor3f(color.r, color.g, color.b);
    glVertex3f(v[0].x, v[0].y, v[0].z);
    glVertex3f(v[1].x, v[1].y, v[1].z);
    glVertex3f(v[2].x, v[2].y, v[2].z);
    glVertex3f(v[3].x, v[3].y, v[3].z);
    glEnd();
}

void DrawBoxLines(const Box& box, const glm::vec3& color = glm::vec3(1.0f))
{
    std::array<glm::vec3, 8> v;
    box.GetCornerPoints(v);

    int indices[] = {
        0, 1,
        1, 3,
        3, 2,
        2, 0,
        0, 4,
        1, 5,
        3, 7,
        2, 6,
        4, 5,
        5, 7,
        7, 6,
        6, 4,
    };

    glBegin(GL_LINES);
    glColor3f(color.r, color.g, color.b);

    for (int i = 0; i < 24; i++)
    {
        glVertex3f(v[indices[i]].x, v[indices[i]].y, v[indices[i]].z);
    }
    glEnd();
}

void DrawPolygon(const glm::vec3* vertices, int numVertices, const glm::vec3& color = glm::vec3(1.0f))
{
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(color.r, color.g, color.b);
    for (int i = 0; i < numVertices; i++)
    {
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    glEnd();
}

void DrawPolygonLines(const glm::vec3* vertices, int numVertices, const glm::vec3& color = glm::vec3(1.0f))
{
    glBegin(GL_LINE_LOOP);
    glColor3f(color.r, color.g, color.b);
    for (int i = 0; i < numVertices; i++)
    {
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    glEnd();
}

void DrawTexturedPolygon(const std::vector<glm::vec3>& vertices, int textureIndex)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureIndex);

    glm::vec2 uvs[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
    };

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < vertices.size(); i++)
    {
        glTexCoord2f(uvs[i].s, uvs[i].t);
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void DrawTexturedFace(const Face& face, Texture texture)
{
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);

    for (const glm::vec3& vertex : face.vertices)
    {
        min = glm::min(min, vertex);
        max = glm::max(max, vertex);
    }

    float width = glm::abs(glm::dot(face.uAxis, max - min));
    float height = glm::abs(glm::dot(face.vAxis, max - min));


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < face.vertices.size(); i++)
    {
        // FIX THIS!!!
        float u = glm::abs(glm::dot(face.uAxis, face.vertices[i] - min) / width);
        float v = glm::abs(glm::dot(face.vAxis, face.vertices[i] - min) / height);

        if (face.uAxis.x == -1 || face.uAxis.z == -1)
        {
            u = 1.0f - u;
        }
        if (face.vAxis.z == -1)
        {
            v = 1.0f - v;
        }


        float pw = 1.0f / texture.width;
        float ph = 1.0f / texture.height;

        u = (u == 0.0f) * pw + u - (u == 1.0f) * pw;
        v = (v == 0.0f) * ph + v - (v == 1.0f) * ph;

        glTexCoord2f(u, v);
        glVertex3f(face.vertices[i].x, face.vertices[i].y, face.vertices[i].z);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

bool RayQuadIntersection(const Ray& ray, const Quad& quad, glm::vec3& point)
{
    glm::vec3 v[4];
    GetQuadVertices(quad, v);

    glm::vec2 barypos;
    float t;
    if (glm::intersectRayTriangle(ray.origin, ray.direction, v[0], v[1], v[2], barypos, t))
    {
        point = ray.origin + ray.direction * t;
        return true;
    }
    else if (glm::intersectRayTriangle(ray.origin, ray.direction, v[0], v[2], v[3], barypos, t))
    {
        point = ray.origin + ray.direction * t;
        return true;
    }
    return false;
}

bool RayPolygonIntersection(const Ray& ray, const glm::vec3* vertices, int numVertices, glm::vec3& point)
{
    for (int i = 0; i < numVertices - 2; i++)
    {
        glm::vec2 barypos;
        float t;
        if (glm::intersectRayTriangle(ray.origin, ray.direction, vertices[0], vertices[i + 1], vertices[i + 2], barypos, t))
        {
            point = ray.origin + ray.direction * t;
            return true;
        }
    }
    return false;
}

void GetQuadLuxels(const Quad& quad, int cols, int rows, int padding, glm::vec3* luxels)
{
    glm::vec3 up    = glm::cross(quad.normal, kWorldRight);
    glm::vec3 right = glm::cross(up, quad.normal);

    float xOffset = (quad.width / (float)cols) * 1.5f;
    float yOffset = (quad.height / (float)rows) * 1.5f;

    glm::vec3* luxel = luxels;

    for (int y = -padding; y < rows + padding; y++)
    {
        for (int x = -padding; x < cols + padding; x++)
        {
            float col = (float)x / (float)cols - xOffset;
            float row = (float)y / (float)rows - yOffset;

            *luxel++ = quad.center + up * quad.height * row - right * quad.width * col;
        }
    }
}

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void UpdateCameraMovement(Camera& camera, Movement& movement, float dt)
{
    glm::vec3 forward = GetForwardVector(GetCameraRotation(camera));
    glm::vec3 right = GetRightVector(GetCameraRotation(camera));

    glm::vec3 acceleration = glm::vec3(0.0f);

    float speedBump = 1.0f;

    const uint8_t* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LSHIFT])
    {
        speedBump = 2.0f;
    }

    if (keys[SDL_SCANCODE_W])
    {
        acceleration += forward;
    }
    if (keys[SDL_SCANCODE_S])
    {
        acceleration -= forward;
    }
    if (keys[SDL_SCANCODE_A])
    {
        acceleration -= right;
    }
    if (keys[SDL_SCANCODE_D])
    {
        acceleration += right;
    }
    if (keys[SDL_SCANCODE_E])
    {
        acceleration += kWorldUp;
    }
    if (keys[SDL_SCANCODE_Q])
    {
        acceleration -= kWorldUp;
    }

    if (glm::length(acceleration) > 0.0f)
    {
        acceleration = glm::normalize(acceleration);
    }

    acceleration *= movement.speed * speedBump;

    glm::vec3 friction = -movement.velocity * movement.friction;
    glm::vec3 totalAcceleration = acceleration + friction;

    movement.velocity += totalAcceleration * dt;
    camera.position += movement.velocity * dt;

    float smoothFactor = 0.1f; // Adjust this value to your liking
    camera.yaw   = Lerp(camera.yaw  , camera.yaw   - mouseDelta.x, smoothFactor);
    camera.pitch = Lerp(camera.pitch, camera.pitch - mouseDelta.y, smoothFactor);
}

GLuint CreateTextureFromImage(const uint32_t* pixels, int width, int height)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

void DrawQuadFromLine(const glm::vec3& v1, const glm::vec3& v2, float floorHeight, float ceilingHeight, const glm::vec3& color = glm::vec3(1.0f))
{
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(v2 - v1, up));

    glm::vec3 v[4] = {
        v1 + up * ceilingHeight,
        v2 + up * ceilingHeight,
        v2 + up * floorHeight,
        v1 + up * floorHeight
    };

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(color.r, color.g, color.b);
    glVertex3f(v[0].x, v[0].y, v[0].z);
    glVertex3f(v[1].x, v[1].y, v[1].z);
    glVertex3f(v[2].x, v[2].y, v[2].z);
    glVertex3f(v[3].x, v[3].y, v[3].z);
    glEnd();
}

// Creates a box that encloses all the vertices of a quad and adds padding
Box GetQuadBoundingBox(const Quad& quad, float padding)
{
    glm::vec3 v[4];
    GetQuadVertices(quad, v);

    Box box;
    box.min = glm::vec3(FLT_MAX);
    box.max = glm::vec3(-FLT_MAX);

    for (int i = 0; i < 4; i++)
    {
        box.min = glm::min(box.min, v[i]);
        box.max = glm::max(box.max, v[i]);
    }

    box.min -= glm::vec3(padding);
    box.max += glm::vec3(padding);

    return box;
}

#include <stdio.h>

#include <fstream>
#include <sstream>
#include <string>

#define BIT(x) (1 << x)

enum CellFlags
{
    CellFlags_None    = 0,
    CellFalgs_Visited = BIT(0),
};

struct Cell
{
    int x;
    int y;
    float floor;
    float ceiling;
    int flags;
    std::array<Cell*, 4> neighbors;
};


std::vector<Quad> CreateQuads(std::vector<Cell>& cells)
{
    std::vector<Quad> quads;

    for (Cell& cell : cells)
    {
        // Ground
        Quad quad;
        quad.center.x = (float)cell.x + 0.5f;
        quad.center.y = cell.floor;
        quad.center.z = (float)cell.y + 0.5f;
        quad.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        quad.width = 1.0f;
        quad.height = 1.0f;
        quads.push_back(quad);

        // Ceiling
        quad.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        quad.center.y = cell.ceiling;
        quads.push_back(quad);

        // Walls
        for (size_t i = 0; i < 4; i++)
        {
            int x = cell.x;
            int y = cell.y;

            glm::ivec2 coords[4] = {
                glm::vec2(x, y - 1),
                glm::vec2(x, y + 1),
                glm::vec2(x + 1, y),
                glm::vec2(x - 1, y),
            };

            glm::vec3 normals[4] = {
                glm::vec3( 0.0f, 0.0f,  1.0f),
                glm::vec3( 0.0f, 0.0f, -1.0f),
                glm::vec3(-1.0f, 0.0f,  0.0f),
                glm::vec3( 1.0f, 0.0f,  0.0f)
            };

            const Cell* other = cell.neighbors[i];

            // Floor & Ceiling
            Quad quad;
            quad.center.x = static_cast<float>(coords[i].x) + (glm::sign(normals[i].x) == 1) * normals[i].x + (glm::abs(normals[i].z) > 0.9f) * 0.5f;
            quad.center.z = static_cast<float>(coords[i].y) + (glm::sign(normals[i].z) == 1) * normals[i].z + (glm::abs(normals[i].x) > 0.9f) * 0.5f;
            quad.normal = normals[i];
            quad.width = 1.0f;

            // When there is an adjacent cell, we need to check if the floor or ceiling is higher. Otherwise, we can just create a wall.
            if (other)
            {
                if (cell.floor < other->floor)
                {
                    float height = other->floor - cell.floor;
                    quad.center.y = cell.floor + height / 2.0f;
                    quad.height = height;
                    quads.push_back(quad);
                }
                
                if (cell.ceiling > other->ceiling)
                {
                    float height = cell.ceiling - other->ceiling;
                    quad.center.y = cell.ceiling - height / 2.0f;
                    quad.height = height;
                    quads.push_back(quad);
                }
            }
            else
            {
                float height = cell.ceiling - cell.floor;
                quad.center.y = cell.floor + height / 2.0f;
                quad.height = height;
                quads.push_back(quad);
            }
        }
    }
    return quads;
}


// Generate radom colors
std::vector<glm::vec3> GenerateColors(size_t count)
{
    std::vector<glm::vec3> colors;
    colors.resize(count);

    for (size_t i = 0; i < count; i++)
    {
        colors[i] = glm::vec3(
            (float)(63 + rand() % 192) / 255.0f,
            (float)(63 + rand() % 192) / 255.0f,
            (float)(63 + rand() % 192) / 255.0f
        );
    }
    return colors;
}


std::vector<Cell> CreateCellsFromData(int* floorData, int* ceilingData, int width, int height)
{
    std::vector<Cell> cells;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Floor
            int f = floorData[y * width + x];
            int c = ceilingData[y * width + x];

            if (f == -1)
            {
                continue;
            }

            Cell& cell = cells.emplace_back();
            cell.x = x;
            cell.y = y;
            cell.floor = (float)f / 32.0f;
            cell.ceiling = (float)c / 32.0f;
            cell.neighbors.fill(nullptr);
        }
    }

    // Second pass: set all the neighbors
    for (Cell& cell : cells)
    {
        int x = cell.x;
        int y = cell.y;

        glm::ivec2 coords[4] = {
            glm::ivec2(x, y - 1),
            glm::ivec2(x, y + 1),
            glm::ivec2(x + 1, y),
            glm::ivec2(x - 1, y),
        };

        for (int i = 0; i < 4; i++)
        {
            for (Cell& other : cells)
            {
                if (other.x == coords[i].x && other.y == coords[i].y)
                {
                    cell.neighbors[i] = &other;
                    break;
                }
            }
        }
    }
    return cells;
}

void CalculateCamerPosition(const std::vector<Cell>& cells, Camera& camera)
{
    glm::vec3 min = glm::vec3(std::numeric_limits<float>().max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>().min());


    for (const Cell& cell : cells)
    {
        min = glm::min(min, glm::vec3(cell.x, cell.floor, cell.y));
        max = glm::max(max, glm::vec3(cell.x, cell.ceiling, cell.y));
    }

    glm::vec3 center = (max + min) / 2.0f;

    float distance = glm::distance(min, max);

    glm::vec3 forward = GetForwardVector(GetCameraRotation(camera));

    camera.position = center - forward * distance;
    camera.position.y = max.y + distance * 0.25f;
    camera.LookAt(center);
}



int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    SDL_Window* window = SDL_CreateWindow("Tremble", 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);
    gladLoadGL();

    Camera camera = {};
    camera.fov = 70.0f;
    camera.aspect = 1280.0f / 720.0f;
    camera.near = 0.1f;
    camera.far = 1000.0f;
    camera.position = glm::vec3(2.0f, 1.0f, 3.0f);

    Movement movement = {};
    movement.speed = 20.0f;
    movement.friction = 6.0f;
    movement.mouseSensitivity = 0.1f;

    const int mwidth = 16;
    const int mheight = 16;
    int floorData[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    int ceilingData[] = {
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    };



    std::vector<Cell> cells = CreateCellsFromData(floorData, ceilingData, mwidth, mheight);
    std::vector<Quad> quads = CreateQuads(cells);
    std::vector<glm::vec3> colors = GenerateColors(quads.size());
    
    const Quad* selectedQuad = nullptr;

    CalculateCamerPosition(cells, camera);

    bool cameraMoveEnabled = false;
    bool running = true;

    bool onButtonDownState = false;
    bool gridSelection = false;
    glm::vec2 minSelection;
    glm::vec2 maxSelection;
    int mx = 0;
    int my = 0;

    /* Loop until the user closes the window */
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
                if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    gridSelection = false;
                }
            }
            else if (ev.type == SDL_EVENT_MOUSE_MOTION)
            {
                mouseDelta.x += (float)ev.motion.xrel;
                mouseDelta.y += (float)ev.motion.yrel;

                int cellSize = 16;
                int w = cellSize * mwidth;
                int h = cellSize * mheight;

                int mw = ev.button.x;
                int mh = ev.button.y;
                mx = mw / cellSize;
                my = mh / cellSize;

                if (onButtonDownState)
                {
                    if (mw >= 0 && mw < w && mh >= 0 && mh < h)
                    {
                        maxSelection = glm::floor(glm::vec2(mw, mh) / (float)cellSize);
                    }
                    else
                    {
                        gridSelection = false;
                    }
                }
            }
            else if (ev.type == SDL_EVENT_MOUSE_WHEEL)
            {
                if (gridSelection)
                {
                    for (int y = (int)minSelection.y; y <= (int)maxSelection.y; y++)
                    {
                        for (int x = (int)minSelection.x; x <= (int)maxSelection.x; x++)
                        {
                            floorData[y * mwidth + x] += static_cast<int>(ev.wheel.y * 8.0f);
                        }
                    }

                    cells.clear();
                    cells = CreateCellsFromData(floorData, ceilingData, mwidth, mheight);
                    quads.clear();
                    quads = CreateQuads(cells);
                }
            }
            else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                if (ev.button.button == SDL_BUTTON_RIGHT)
                {
                    cameraMoveEnabled = true;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }

                if (ev.button.button == SDL_BUTTON_LEFT)
                {
                    int cellSize = 16;
                    int w = cellSize * mwidth;
                    int h = cellSize * mheight;

                    int mw = ev.button.x;
                    int mh = ev.button.y;

                    if (!onButtonDownState)
                    {
                        gridSelection = true;

                        if (mw >= 0 && mw < w && mh >= 0 && mh < h)
                        {
                            onButtonDownState = true;
                            minSelection = glm::floor(glm::vec2(mw, mh) / (float)cellSize);
                            maxSelection = minSelection;
                        }
                    }
                }
                
            }
            else if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                if (ev.button.button == SDL_BUTTON_RIGHT)
                {
                    cameraMoveEnabled = false;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }

                if (ev.button.button == SDL_BUTTON_LEFT)
                {
                    int cellSize = 16;
                    int w = cellSize * mwidth;
                    int h = cellSize * mheight;

                    int mw = ev.button.x;
                    int mh = ev.button.y;
                    onButtonDownState = false;

                    if (mw >= 0 && mw < w && mh >= 0 && mh < h)
                    {
                        maxSelection = glm::floor(glm::vec2(mw, mh) / (float)cellSize);
                    }
                }
            }
        }

        // Calculate delta time
        static uint64_t lastTime = SDL_GetPerformanceCounter();
        uint64_t currentTime = SDL_GetPerformanceCounter();
        float deltaTime = float(currentTime - lastTime) / SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        if (cameraMoveEnabled)
        {
            UpdateCameraMovement(camera, movement, deltaTime);
        }

        glm::mat4 projectionMatrix = GetProjection(camera);
        glm::mat4 viewMatrix = GetView(camera);

        glEnable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(projectionMatrix));
                
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(viewMatrix));
        
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, -1.0f);
        int i = 0;
        for (const Quad& quad : quads)
        {
            i = (i + 1) % colors.size();
            DrawQuad(quad, glm::vec3(0.8f));
        }

        glDepthFunc(GL_LEQUAL);

        // Highlight selected quad
        if (gridSelection)
        {
            for (int y = (int)minSelection.y; y <= (int)maxSelection.y; y++)
            {
                if (y >= mheight) break;

                for (int x = (int)minSelection.x; x <= (int)maxSelection.x; x++)
                {
                    if (x >= mwidth) break;

                    int index = y * mwidth + x;

                    for (const Cell& cell : cells)
                    {
                        if (cell.x == x && cell.y == y)
                        {
                            Quad quad;
                            quad.center.x = (float)cell.x + 0.5f;
                            quad.center.y = cell.floor;
                            quad.center.z = (float)cell.y + 0.5f;
                            quad.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                            quad.width = 1.0f;
                            quad.height = 1.0f;
                            DrawQuad(quad, glm::vec3(1.0f, 0.41f, 0.04f));
                            break;
                        }
                    }

                }
            }
        }

        if (mx >= 0 && mx < mwidth && my >= 0 && my < mheight)
        {
            for (const Cell& cell : cells)
            {
                if (cell.x == mx && cell.y == my)
                {
                    Quad quad;
                    quad.center.x = (float)cell.x + 0.5f;
                    quad.center.y = cell.floor;
                    quad.center.z = (float)cell.y + 0.5f;
                    quad.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    quad.width = 1.0f;
                    quad.height = 1.0f;
                    DrawQuad(quad, glm::vec3(1.0f, 0.41f, 0.04f));
                    break;
                }
            }
        }

        glDepthFunc(GL_LESS);
        glDisable(GL_POLYGON_OFFSET_FILL);


        for (const Quad& quad : quads)
        {
            glm::vec3 cameraToQuad = quad.center - camera.position;
            if (glm::dot(cameraToQuad, quad.normal) > 0.0f) continue;

            DrawQuadLines(quad, glm::vec3(0.6f));
        }

        glm::mat4 ortho = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, -1.0f, 1.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(ortho));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);

        // Draw grid

        const int cellWidth = 16;
        const int cellHeight = 16;

        int w = cellWidth * mwidth;
        int h = cellHeight * mheight;

        // Draw background
        glBegin(GL_TRIANGLE_FAN);
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2i(0, 0);
            glVertex2i(0, h);
            glVertex2i(w, h);
            glVertex2i(w, 0);
        glEnd();


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Draw grid selection
        if (gridSelection)
        {
            for (int y = (int)minSelection.y; y <= (int)maxSelection.y; y++)
            {
                if (y >= mheight) break;

                for (int x = (int)minSelection.x; x <= (int)maxSelection.x; x++)
                {
                    if (x >= mwidth) break;

                    glBegin(GL_TRIANGLE_FAN);
                    glColor4f(1.00, 0.41, 0.04, 0.75f);
                    glVertex2i(x * cellWidth, y * cellHeight);
                    glVertex2i(x * cellWidth, (y + 1) * cellHeight);
                    glVertex2i((x + 1) * cellWidth, (y + 1) * cellHeight);
                    glVertex2i((x + 1) * cellWidth, y * cellHeight);
                    glEnd();
                }
            }
        }
        glDisable(GL_BLEND);


        // Draw grid
        glBegin(GL_LINES);
        glColor3f(0.4f, 0.4f, 0.4f);
        for (int x = 0; x <= w; x += cellWidth)
        {
            glVertex2i(x, 0);
            glVertex2i(x, h);
        }
        for (int y = 0; y <= h; y += cellHeight)
        {
            glVertex2i(0, y);
            glVertex2i(w, y);
        }
        glEnd();

        glEnable(GL_DEPTH_TEST);

        SDL_GL_SwapWindow(window);

        mouseDelta = glm::vec2(0.0f);
    }
    return 0;
}
