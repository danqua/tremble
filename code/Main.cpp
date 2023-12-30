#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/quaternion.hpp>

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

glm::vec3 GetForwardVector(const glm::vec3& rotation)
{
    return glm::normalize(glm::quat(glm::radians(rotation)) * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 GetRightVector(const glm::vec3& rotation)
{
    return glm::normalize(glm::quat(glm::radians(rotation)) * glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 GetUpVector(const glm::vec3& rotation)
{
    return glm::normalize(glm::quat(glm::radians(rotation)) * glm::vec3(0.0f, 1.0f, 0.0f));
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
    return glm::lookAt(camera.position, camera.position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
}


void GetQuadVertices(const Quad& quad, glm::vec3 vertices[4])
{
    glm::vec3 up    = glm::cross(quad.normal, glm::vec3(1.0f, 0.0f, 0.0f));
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

void GetQuadLuxels(const Quad& quad, int cols, int rows, int padding, glm::vec3* luxels)
{
    glm::vec3 up    = glm::cross(quad.normal, glm::vec3(1.0f, 0.0f, 0.0f));
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

    if (keys[GLFW_KEY_LEFT_SHIFT])
    {
        movement.speed = 20.0f;
    }
    else
    {
        movement.speed = 10.0f;
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
        acceleration.y += 1.0f;
    }
    if (keys[GLFW_KEY_Q])
    {
        acceleration.y -= 1.0f;
    }

    if (glm::length(acceleration) > 0.0f)
    {
        acceleration = glm::normalize(acceleration);
    }

    acceleration *= movement.speed;

    glm::vec3 friction = -movement.velocity * movement.friction;
    glm::vec3 totalAcceleration = acceleration + friction;

    movement.velocity += totalAcceleration * dt;
    camera.position += movement.velocity * dt;

    float smoothFactor = 0.1f; // Adjust this value to your liking
    camera.yaw = Lerp(camera.yaw, camera.yaw - mouseDelta.x, smoothFactor);
    camera.pitch = Lerp(camera.pitch, camera.pitch - mouseDelta.y, smoothFactor);
}

int main(int argc, char** argv)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

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
    camera.far = 100.0f;
    camera.position = glm::vec3(0.0f, 0.0f, 4.0f);

    Movement movement = {};
    movement.speed = 10.0f;
    movement.friction = 6.0f;
    movement.mouseSensitivity = 0.1f;

    Quad quad1 = {};
    quad1.normal = glm::vec3(0.0f, 0.0f, 1.0f);
    quad1.width = 1.0f;
    quad1.height = 1.0f;

    Quad quad2 = {};
    quad2.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    quad2.center = glm::vec3(0.0f, -0.5f, 0.5f);
    quad2.width = 1.0f;
    quad2.height = 1.0f;

    const int numQuads = 2;
    Quad quads[numQuads] = { quad1, quad2 };

    Light light = {};
    light.position = glm::vec3(0.0f, 0.0f, 2.0f);

    int quadLuxelCols = 4;
    int quadLuxelRows = 4;
    int quadLuxelPadding = 1;
    int quadNumLuxels = (quadLuxelCols + quadLuxelPadding * 2) * (quadLuxelRows + quadLuxelPadding * 2);

    glm::vec3* luxels = new glm::vec3[quadNumLuxels * numQuads];

    glm::vec3* luxelWalker = luxels;
    for (int i = 0; i < numQuads; i++)
    {
        GetQuadLuxels(quads[i], quadLuxelCols, quadLuxelRows, quadLuxelPadding, luxelWalker);
        luxelWalker += quadNumLuxels;
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

        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(projectionMatrix));
                
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixf(glm::value_ptr(viewMatrix));

        for (int i = 0; i < numQuads; i++)
        {
            DrawQuadLines(quads[i]);
        }
        
        DrawPoint(light.position, glm::vec3(1.0f, 1.0f, 0.0f), 8.0f);

        Ray ray = {};
        ray.origin = light.position;

        glm::vec3* luxelWalker = luxels;
        luxelWalker += quadNumLuxels;
        for (int i = 0; i < quadNumLuxels; i++)
        {
            ray.direction = glm::normalize(luxelWalker[i] - light.position);

            glm::vec3 point;
            if (RayQuadIntersection(ray, quads[0], point))
            {
                DrawLine(ray.origin, luxelWalker[i]);
                DrawPoint(luxelWalker[i], glm::vec3(0.0f, 1.0f, 0.0f), 8.0f);
            }
            else
            {
                DrawLine(ray.origin, luxelWalker[i], glm::vec3(1.0f, 0.0f, 0.0f));
                DrawPoint(luxelWalker[i], glm::vec3(1.0f, 0.0f, 0.0f), 8.0f);
            }
        }
        

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        mouseDelta = glm::vec2(0.0f);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}