#include <GLFW/glfw3.h>
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

bool keys[1024];
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
    glm::vec3 up    = glm::cross(quad.normal, kWorldRight);
    glm::vec3 right = glm::cross(up, quad.normal);

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

    if (keys[GLFW_KEY_LEFT_SHIFT])
    {
        speedBump = 2.0f;
    }

    if (keys[GLFW_KEY_W])
    {
        acceleration += forward;
    }
    if (keys[GLFW_KEY_S])
    {
        acceleration -= forward;
    }
    if (keys[GLFW_KEY_A])
    {
        acceleration -= right;
    }
    if (keys[GLFW_KEY_D])
    {
        acceleration += right;
    }
    if (keys[GLFW_KEY_E])
    {
        acceleration += kWorldUp;
    }
    if (keys[GLFW_KEY_Q])
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

bool PointInPolygon(const glm::vec3& point, const glm::vec3* vertices, int numVertices);

#include <stdio.h>

#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char** argv)
{
    GLFWwindow* window;

    if (!glfwInit())
    {
        return -1;
    }

    int windowWidth = 640;
    int windowHeight = 480;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(windowWidth, windowHeight, "Hello World", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_ESCAPE)
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            keys[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
        }
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        static double lastX = xpos;
        static double lastY = ypos;
        static bool firstMouse = true;

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        double deltaX = xpos - lastX;
        double deltaY = ypos - lastY;

        lastX = xpos;
        lastY = ypos;

        mouseDelta.x = (float)deltaX;
        mouseDelta.y = (float)deltaY;
    });


    Camera camera = {};
    camera.fov = 70.0f;
    camera.aspect = 640.0f / 480.0f;
    camera.near = 0.1f;
    camera.far = 1000.0f;
    camera.position = glm::vec3(2.0f, 1.0f, -3.0f);

    Movement movement = {};
    movement.speed = 10.0f;
    movement.friction = 6.0f;
    movement.mouseSensitivity = 0.1f;

    struct Sector
    {
        int firstWall;
        int numWalls;
        float floorHeight;
        float ceilingHeight;
    };

    struct Wall
    {
        int v[2];
        int sector;
    };

    std::vector<glm::vec2> wallVertices = {
        glm::vec2( 0.0f, 0.0f),
        glm::vec2( 4.0f, 0.0f),
        glm::vec2( 5.0f, 2.0f),
        glm::vec2( 7.0f, 2.0f),
        glm::vec2( 7.0f, 4.0f),
        glm::vec2( 5.0f, 4.0f),
        glm::vec2( 4.0f, 6.0f),
        glm::vec2( 0.0f, 6.0f),
        glm::vec2( 9.0f, 1.0f),
        glm::vec2(11.0f, 1.0f),
        glm::vec2(11.0f, 3.0f),
        glm::vec2( 9.0f, 3.0f)
    };

    std::vector<Sector> sectors = {
        {  0, 6,  0.0f, 3.0f },
        {  6, 4, 0.25f, 2.0f },
        { 10, 4,  0.5f, 3.0f },
        { 14, 4,  0.75f, 3.0f }
    };

    std::vector<Wall> walls = {
        { {  0,  1 }, -1 },
        { {  1,  2 }, -1 },
        { {  2,  5 },  1 },
        { {  5,  6 }, -1 },
        { {  6,  7 }, -1 },
        { {  7,  0 }, -1 },
        { {  2,  3 }, -1 },
        { {  3,  4 },  2 },
        { {  4,  5 }, -1 },
        { {  5,  2 },  0 },
        { {  3,  8 }, -1 },
        { {  8, 11 },  3 },
        { { 11,  4 }, -1 },
        { {  4,  3 },  1 },
        { {  8,  9 }, -1 },
        { {  9, 10 }, -1 },
        { { 10, 11 }, -1 },
        { { 11,  8 },  2 },
    };

    glm::vec3 randomColors[16] = {
        glm::vec3(0.6f, 0.8f, 0.8f),
        glm::vec3(0.7f, 0.7f, 1.0f),
        glm::vec3(0.6f, 0.8f, 0.7f),
        glm::vec3(0.7f, 0.9f, 0.9f),
        glm::vec3(0.7f, 1.0f, 1.0f),
        glm::vec3(0.8f, 0.6f, 0.8f),
        glm::vec3(0.8f, 0.6f, 1.0f),
        glm::vec3(1.0f, 0.6f, 1.0f),
        glm::vec3(0.8f, 0.6f, 0.8f),
        glm::vec3(0.9f, 0.5f, 0.7f),
        glm::vec3(0.8f, 0.8f, 0.7f),
        glm::vec3(0.8f, 0.7f, 0.5f),
        glm::vec3(0.9f, 0.7f, 0.9f),
        glm::vec3(0.5f, 1.0f, 0.7f),
        glm::vec3(0.7f, 1.0f, 0.8f),
        glm::vec3(0.8f, 0.9f, 0.7f)
    };

    auto PointInSector = [=](const glm::vec3& point, const Sector& sector) {

        std::vector<glm::vec3> vertices;

        for (int i = 0; i < sector.numWalls; ++i)
        {
            const Wall& wall = walls[sector.firstWall + i];

            glm::vec2 v[2] = {
                wallVertices[wall.v[0]],
                wallVertices[wall.v[1]]
            };

            glm::vec3 v3[2] = {
                glm::vec3(v[0].x, 0.0f, -v[0].y),
                glm::vec3(v[1].x, 0.0f, -v[1].y)
            };

            vertices.push_back(v3[0]);
        }

        return PointInPolygon(point, vertices.data(), vertices.size());
    };

    int wallIndex = 0;

    int drawnSectors = 0;
    auto DrawSector = [&](const Sector& sector)
    {

        for (int i = 0; i < sector.numWalls; ++i)
        {
            const Wall& wall = walls[sector.firstWall + i];
            glm::vec2 v2[2] = { wallVertices[wall.v[0]], wallVertices[wall.v[1]] };
            glm::vec3 v3[2] = { glm::vec3(v2[0].x, 0.0f, -v2[0].y), glm::vec3(v2[1].x, 0.0f, -v2[1].y) };

            glm::vec3 normal = glm::normalize(glm::cross(v3[1] - v3[0], glm::vec3(0.0f, 1.0f, 0.0f)));

            glm::vec3 color = randomColors[wallIndex++ % 16];
            color = glm::vec3(glm::dot(glm::vec3(0.8f, 0.65f, 0.9f), glm::abs(normal)));
            if (wall.sector != -1)
            {
                const Sector& otherSector = sectors[wall.sector];
                if (otherSector.floorHeight > sector.floorHeight)
                {
                    DrawQuadFromLine(v3[0], v3[1], sector.floorHeight, otherSector.floorHeight, color);
                }
                if (otherSector.ceilingHeight < sector.ceilingHeight)
                {
                    DrawQuadFromLine(v3[0], v3[1], otherSector.ceilingHeight, sector.ceilingHeight, color);
                }
                continue;
            }

            DrawQuadFromLine(v3[0], v3[1], sector.floorHeight, sector.ceilingHeight, color);
        }

        std::vector<glm::vec3> vertices;
        for (int i = 0; i < sector.numWalls; ++i)
        {
            const Wall& wall = walls[sector.firstWall + i];
            glm::vec2 v2[2] = { wallVertices[wall.v[0]], wallVertices[wall.v[1]] };
            glm::vec3 v3[2] = { glm::vec3(v2[0].x, sector.floorHeight, -v2[0].y), glm::vec3(v2[1].x, sector.floorHeight, -v2[1].y) };
            vertices.push_back(v3[0]);
        }

        glm::vec3 color = glm::vec3(glm::dot(glm::vec3(0.8f, 0.65f, 0.9f), glm::vec3(0.0f, 1.0f, 0.0f)));

        
        DrawPolygon(vertices.data(), vertices.size(), color);

        std::reverse(vertices.begin(), vertices.end());
        for (glm::vec3& vertex : vertices)
        {
            vertex.y = sector.ceilingHeight;
        }
        DrawPolygon(vertices.data(), vertices.size(), color);

        drawnSectors++;
    };
    
    std::function<void(const Sector&, std::vector<const Sector*>&)> MarkSectorsVisible = [&](const Sector& sector, std::vector<const Sector*>& visibleSectors)
    {
        visibleSectors.push_back(&sector);

        for (int i = 0; i < sector.numWalls; ++i)
        {
            const Wall& wall = walls[sector.firstWall + i];

            glm::vec2 v2[2] = {
                wallVertices[wall.v[0]],
                wallVertices[wall.v[1]]
            };

            glm::vec3 v3[2] = {
                glm::vec3(v2[0].x, 0.0f, -v2[0].y),
                glm::vec3(v2[1].x, 0.0f, -v2[1].y)
            };

            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(v3[1] - v3[0], up));
            glm::vec3 normal = glm::cross(right, up);
            float ceilingHeight = 2.0f;
            float floorHeight = 0.0f;

            glm::vec3 v[4] = {
                v3[0] + up * ceilingHeight,
                v3[1] + up * ceilingHeight,
                v3[1] + up * floorHeight,
                v3[0] + up * floorHeight
            };

            Box box(v, 4);
            box.Expand(right * 0.05f);

            if (wall.sector != -1)
            {
                if (std::find(visibleSectors.begin(), visibleSectors.end(), &sectors[wall.sector]) != visibleSectors.end())
                {
                    continue;
                }

                glm::mat4 projectionMatrix = GetProjection(camera);
                glm::mat4 viewMatrix = GetView(camera);
                Frustum frustum(projectionMatrix * viewMatrix);

                if (frustum.IntersectsBox(box))
                {
                    MarkSectorsVisible(sectors[wall.sector], visibleSectors);
                }
            }
        }
    };
    

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        static double lastTime = glfwGetTime();
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        UpdateCameraMovement(camera, movement, deltaTime);

        glm::mat4 projectionMatrix = GetProjection(camera);
        glm::mat4 viewMatrix = GetView(camera);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(projectionMatrix));
                
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(viewMatrix));
        
        glViewport(0, 0, windowWidth, windowHeight);

        drawnSectors = 0;
        wallIndex = 0;


        std::vector<const Sector*> visibleSectors;
        for (const Sector& sector : sectors)
        {
            if (PointInSector(camera.position, sector))
            {
                MarkSectorsVisible(sector, visibleSectors);
                break;
            }
        }


        for (const Sector* sector : visibleSectors)
        {
            DrawSector(*sector);
        }
        printf("Sectors: %d\n", drawnSectors);


        glfwSwapBuffers(window);

        mouseDelta = glm::vec2(0.0f);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}



bool PointInPolygon(const glm::vec3& point, const glm::vec3* vertices, int numVertices)
{
    glm::vec3 p = point;

    int i, j, c = 0;
    for (i = 0, j = numVertices - 1; i < numVertices; j = i++)
    {
        if (((vertices[i].z > p.z) != (vertices[j].z > p.z)) &&
            (p.x < (vertices[j].x - vertices[i].x) * (p.z - vertices[i].z) / (vertices[j].z - vertices[i].z) + vertices[i].x))
        {
            c = !c;
        }
    }
    return c;
}