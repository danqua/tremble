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

#include "Math/Box.h"

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

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
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
    camera.position = glm::vec3(0.0f, 1.0f, 3.0f);

    Movement movement = {};
    movement.speed = 10.0f;
    movement.friction = 6.0f;
    movement.mouseSensitivity = 0.1f;

    Math::Box box(glm::vec3(0.0f), 2.0f);
    std::array<glm::vec3, 8> boxPoints;
    box.GetCornerPoints(boxPoints);

    std::vector<Face> faces;

    struct SectorInfo
    {
        int wallIndex;
        int numWalls;
        int numPortals;
        int* portals;

        bool IsPortal(int index) const
        {
            for (int i = 0; i < numPortals; i++)
            {
                if (portals[i] == index)
                {
                    return true;
                }
            }
            return false;
        }
    };

    struct WallInfo
    {
        int x1;
        int y1;
        int x2;
        int y2;
    };

    std::vector<SectorInfo> sectorInfos;
    std::vector<WallInfo> wallInfos;

    enum class ParseState
    {
        None,
        Sector,
        Wall,
    };

    ParseState parseState = ParseState::None;


    std::ifstream fs("map.txt");
    
    std::string line;

    while (std::getline(fs, line))
    {
        if (line.find("[sectors]") != std::string::npos)
        {
            parseState = ParseState::Sector;
        }
        else if (line.find("[walls]") != std::string::npos)
        {
            parseState = ParseState::Wall;
        }
        else if (line[0] == '#')
        {
            continue;
        }
        else
        {
            if (line.empty())
            {
                continue;
            }
            if (parseState == ParseState::Sector)
            {
                SectorInfo& sectorInfo = sectorInfos.emplace_back();
                std::stringstream ss(line);
                ss >> sectorInfo.wallIndex >> sectorInfo.numWalls >> sectorInfo.numPortals;

                if (sectorInfo.numPortals > 0)
                {
                    sectorInfo.portals = new int[sectorInfo.numPortals];
                    for (int i = 0; i < sectorInfo.numPortals; i++)
                    {
                        ss >> sectorInfo.portals[i];
                    }
                }
            }
            else if (parseState == ParseState::Wall)
            {
                WallInfo& wallInfo = wallInfos.emplace_back();
                std::stringstream ss(line);
                ss >> wallInfo.x1 >> wallInfo.y1 >> wallInfo.x2 >> wallInfo.y2;
            }
        }
    }

    for (const SectorInfo& sectorInfo : sectorInfos)
    {
        std::vector<Face> sectorFaces;
        for (int i = 0; i < sectorInfo.numWalls; i++)
        {
            if (sectorInfo.IsPortal(sectorInfo.wallIndex + i))
            {
                continue;
            }

            Face& face = sectorFaces.emplace_back();
            const WallInfo& wallInfo = wallInfos[sectorInfo.wallIndex + i];

            glm::vec3 v1 = glm::vec3(wallInfo.x1, 0, wallInfo.y1);
            glm::vec3 v2 = glm::vec3(wallInfo.x2, 0, wallInfo.y2);
            glm::vec3 v3 = glm::vec3(wallInfo.x2, 3, wallInfo.y2);
            glm::vec3 v4 = glm::vec3(wallInfo.x1, 3, wallInfo.y1);

            face.vertices.push_back(v1);
            face.vertices.push_back(v2);
            face.vertices.push_back(v3);
            face.vertices.push_back(v4);
            face.normal = glm::normalize(glm::cross(face.vertices[1] - face.vertices[0], face.vertices[2] - face.vertices[0]));
            face.uAxis = glm::normalize(glm::cross(kWorldUp, face.normal));
            face.vAxis = glm::normalize(glm::cross(face.normal, face.uAxis));
        }

        // Skip floor/ceiling generation for now

        // Create floor and ceiling for the sector
        Face floorFace;
        floorFace.normal = glm::vec3(0.0f, 1.0f,  0.0f);
        floorFace.uAxis  = glm::vec3(1.0f, 0.0f,  0.0f);
        floorFace.vAxis  = glm::vec3(0.0f, 0.0f, -1.0f);

        Face ceilingFace;
        ceilingFace.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        ceilingFace.uAxis  = glm::vec3(1.0f,  0.0f, 0.0f);
        ceilingFace.vAxis  = glm::vec3(0.0f,  0.0f, 1.0f);

        for (const Face& face : sectorFaces)
        {
            floorFace.vertices.push_back(face.vertices[0]);
            ceilingFace.vertices.push_back(face.vertices[0] + glm::vec3(0.0f, 3.0f, 0.0f));
        }

        // revert the order of the ceiling face
        std::reverse(floorFace.vertices.begin(), floorFace.vertices.end());

        sectorFaces.push_back(floorFace);
        sectorFaces.push_back(ceilingFace);

        faces.insert(faces.end(), sectorFaces.begin(), sectorFaces.end());
    }

    Light light = {};
    light.position = glm::vec3(3.0f, 2.0f, 3.0f);
    light.color = glm::vec3(1.0f);
    light.intensity = 4.0f;

    std::vector<Image> images;
    std::vector<Texture> textures;

    int faceIndex = 0;
    for (const Face& face : faces)
    {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);

        for (const glm::vec3& vertex : face.vertices)
        {
            min = glm::min(min, vertex);
            max = glm::max(max, vertex);
        }

        float width  = glm::abs(glm::dot(face.uAxis, max - min));
        float height = glm::abs(glm::dot(face.vAxis, max - min));

        std::vector<glm::vec3> luxels;


        float unit = 0.125f;

        int texWidth = 0;
        int texHeight = 0;


        for (float v = -unit; v < height + unit; v += unit)
        {
            for (float u = -unit; u < width + unit; u += unit)
            {
                glm::vec3 luxel = face.vertices[0] + (face.uAxis * u + face.vAxis * v);
                luxels.push_back(luxel);

                if (texHeight == 0)
                {
                    texWidth++;
                }
            }
            texHeight++;
        }

        // Create image for luxels
        Image& image = images.emplace_back();
        image.width = texWidth;
        image.height = texHeight;
        image.pixels = new uint32_t[texWidth * texHeight];

        int pixelIndex = 0;
        for (const glm::vec3& luxel : luxels)
        {
            Ray ray = {};
            ray.origin = light.position;
            ray.direction = luxel - light.position;

            glm::vec3 point;
            if (RayPolygonIntersection(ray, face.vertices.data(), face.vertices.size(), point))
            {
                float attenuation = 1.0f / glm::dot(point - light.position, point - light.position) * light.intensity;
                uint32_t color = glm::packUnorm4x8(glm::vec4(light.color * attenuation, 1.0f));
                image.pixels[pixelIndex] = color;
            }
            else
            {
                float attenuation = 1.0f / glm::dot(luxel - light.position, luxel - light.position) * light.intensity;
                uint32_t color = glm::packUnorm4x8(glm::vec4(light.color * attenuation, 1.0f));
                image.pixels[pixelIndex] = color;
                //pixels[pixelIndex] = glm::packUnorm4x8(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            }
            pixelIndex++;
        }

        // Save texture to file
        //char filename[256];
        //sprintf(filename, "textures/%d.png", faceIndex++);
        //stbi_write_png(filename, texWidth, texHeight, 4, pixels.data(), texWidth * 4);
        /*
        Texture& texture = textures.emplace_back();
        texture.width = texWidth;
        texture.height = texHeight;
        texture.id = CreateTextureFromImage(pixels.data(), texWidth, texHeight);
        */
    }

    // Create textures from images
    for (const Image& image : images)
    {
        Texture& texture = textures.emplace_back();
        texture.width = image.width;
        texture.height = image.height;
        texture.id = CreateTextureFromImage(image.pixels, image.width, image.height);
    }

    stbrp_rect* rects = new stbrp_rect[images.size()];
    for (auto i = 0; i < images.size(); i++)
    {
        rects[i].id = i;
        rects[i].w = images[i].width;
        rects[i].h = images[i].height;
    }

    int numNodes = 1024;

    stbrp_context context;
    stbrp_node* nodes = new stbrp_node[numNodes];
    stbrp_init_target(&context, 1024, 1024, nodes, numNodes);
    stbrp_pack_rects(&context, rects, images.size());

    uint32_t* atlas = new uint32_t[1024 * 1024];
    for (auto i = 0; i < 1024 * 1024; i++)
    {
        atlas[i] = glm::packUnorm4x8(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    for (auto i = 0; i < images.size(); i++)
    {
        if (rects[i].was_packed)
        {
            for (auto y = 0; y < images[i].height; y++)
            {
                for (auto x = 0; x < images[i].width; x++)
                {
                    atlas[(rects[i].y + y) * 1024 + (rects[i].x + x)] = images[i].pixels[y * images[i].width + x];
                }
            }
        }
    }

    stbi_write_png("textures/atlas.png", 1024, 1024, 4, atlas, 1024 * 4);
    //stbi_write_jpg("textures/atlas.jpg", 1024, 1024, 4, atlas, 100);

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
        
        /*
        for (const glm::vec3& luxel : luxels)
        {
            Ray ray = {};
            ray.origin = light.position;
            ray.direction = luxel - light.position;

            glm::vec3 point;
            if (RayPolygonIntersection(ray, face.vertices.data(), face.vertices.size(), point))
            {
                float attenuation = 1.0f / glm::dot(point - light.position, point - light.position);
                DrawPoint(point, glm::vec3(attenuation), 10.0f);
            }
        }
        */
        // Draw surface
        int textureIndex = 0;
        for (const Face& face : faces)
        {


            /*if (textureIndex == 5)
            {
                DrawPolygonLines(face.vertices.data(), face.vertices.size(), glm::vec3(1.0f, 1.0f, 0.0f));
            }*/
            DrawTexturedFace(face, textures[textureIndex]);
            //DrawPolygonLines(face.vertices.data(), face.vertices.size());
            textureIndex++;
            //DrawTexturedPolygon(face.vertices, textures[textureIndex++]); 
            //DrawPolygonLines(face.vertices.data(), face.vertices.size());

        }
        

        

        DrawPoint(light.position, light.color, 10.0f);

        DrawPoints(boxPoints.data(), boxPoints.size(), glm::vec3(1.0f, 0.0f, 1.0f), 10.0f);

        glfwSwapBuffers(window);

        mouseDelta = glm::vec2(0.0f);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}