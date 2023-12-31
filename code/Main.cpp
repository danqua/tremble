#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <vector>

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

void DrawTexturedFace(const Face& face, GLuint texture)
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
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < face.vertices.size(); i++)
    {
        // FIX THIS!!!
        float u = glm::abs(glm::dot(face.uAxis, face.vertices[i] - min) / width);
        float v = glm::abs(glm::dot(face.vAxis, face.vertices[i] - min) / height);

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

#include <stdio.h>

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


    std::vector<Face> faces;

    FILE* fp = fopen("map.txt", "rb");


    // Read the text file line by line
    char line[1024];
    while (fgets(line, 1024, fp))
    {
        if (line[0] == '#')
        {
            continue;
        }

        int x1, y1, x2, y2;
        sscanf(line, "%d %d %d %d", &x1, &y1, &x2, &y2);

        Face& face = faces.emplace_back();

        glm::vec3 v1 = glm::vec3(x1, 0, y1);
        glm::vec3 v2 = glm::vec3(x2, 0, y2);
        glm::vec3 v3 = glm::vec3(x2, 3, y2);
        glm::vec3 v4 = glm::vec3(x1, 3, y1);
        
        face.vertices.push_back(v1);
        face.vertices.push_back(v2);
        face.vertices.push_back(v3);
        face.vertices.push_back(v4);
        face.normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
        face.uAxis = glm::normalize(glm::cross(kWorldUp, face.normal));
        face.vAxis = glm::normalize(glm::cross(face.normal, face.uAxis));
    }

    // Create floor/ceiling face
    {
        Face floorFace;
        floorFace.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        floorFace.uAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        floorFace.vAxis = glm::vec3(0.0f, 0.0f, 1.0f);

        Face ceilingFace;
        ceilingFace.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        ceilingFace.uAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        ceilingFace.vAxis = glm::vec3(0.0f, 0.0f, 1.0f);

        for (const Face& face : faces)
        {
            floorFace.vertices.push_back(face.vertices[0]);
            ceilingFace.vertices.push_back(face.vertices[0] + glm::vec3(0.0f, 3.0f, 0.0f));
        }

        // revert the order of the ceiling face
        //std::reverse(floorFace.vertices.begin(), floorFace.vertices.end());

        faces.push_back(floorFace);
        faces.push_back(ceilingFace);
    }

    fclose(fp);

    Light light = {};
    light.position = glm::vec3(2.0f, 1.5f, 3.0f);
    light.color = glm::vec3(1.0f);
    light.intensity = 2.0f;

    std::vector<GLuint> textures;

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
        float unit = 0.25f;

        int texWidth = 0;
        int texHeight = 0;


        for (float v = 0; v < height; v += unit)
        {
            for (float u = 0; u < width; u += unit)
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

        // Create texture for luxels
        std::vector<uint32_t> pixels(texWidth * texHeight);
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
                uint32_t color = glm::packUnorm4x8(glm::vec4(glm::vec3(attenuation), 1.0f));
                pixels[pixelIndex] = color;
            }
            else
            {
                float attenuation = 1.0f / glm::dot(luxel - light.position, luxel - light.position) * light.intensity;
                uint32_t color = glm::packUnorm4x8(glm::vec4(glm::vec3(attenuation), 1.0f));
                pixels[pixelIndex] = color;
                //pixels[pixelIndex] = glm::packUnorm4x8(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            }
            pixelIndex++;
        }

        // Save texture to file
        char filename[256];
        sprintf(filename, "textures/%d.png", faceIndex++);
        stbi_write_png(filename, texWidth, texHeight, 4, pixels.data(), texWidth * 4);

        GLuint& texture = textures.emplace_back();
        texture = CreateTextureFromImage(pixels.data(), texWidth, texHeight);
    }

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
        //glEnable(GL_CULL_FACE);
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
            //DrawTexturedPolygon(face.vertices, textures[textureIndex++]); 
            DrawTexturedFace(face, textures[textureIndex++]);
            //DrawPolygonLines(face.vertices.data(), face.vertices.size());
        }
        

        

        DrawPoint(light.position, light.color, 10.0f);

        glfwSwapBuffers(window);

        mouseDelta = glm::vec2(0.0f);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}