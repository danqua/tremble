#include "Intersection.h"
#include <glm/gtx/component_wise.hpp>

bool Math::Intersects(const Box& a, const Box& b)
{
    if (a.isValid && b.isValid)
    {
        return glm::compMax(glm::min(a.max, b.max) - glm::max(a.min, b.min)) >= 0.0f;
    }
    return false;
}

bool Math::Intersects(const Sphere& a, const Sphere& b)
{
    if (!std::isnan(a.radius) && !std::isnan(b.radius))
    {
        return glm::distance(a.center, b.center) <= a.radius + b.radius;
    }
    return false;
}

bool Math::Intersects(const Box& a, const Sphere& b)
{
    if (a.isValid && !std::isnan(b.radius))
    {
        return glm::distance(b.center, glm::clamp(b.center, a.min, a.max)) <= b.radius;
    }
    return false;
}

bool Math::Intersects(const Sphere& a, const Box& b)
{
    return Intersects(b, a);
}

bool Math::Intersects(const Ray& ray, const Box& box)
{
    if (box.isValid)
    {
        glm::vec3 invDir = 1.0f / ray.direction;
        glm::vec3 t0 = (box.min - ray.origin) * invDir;
        glm::vec3 t1 = (box.max - ray.origin) * invDir;
        glm::vec3 tmin = glm::min(t0, t1);
        glm::vec3 tmax = glm::max(t0, t1);
        float t0max = glm::compMax(tmin);
        float t1min = glm::compMin(tmax);
        return t0max <= t1min;
    }
    return false;
}

bool Math::Intersects(const Box& box, const Ray& ray)
{
    return Intersects(ray, box);
}

bool Math::Intersects(const Ray& ray, const Sphere& sphere)
{
    if (!std::isnan(sphere.radius))
    {
        glm::vec3 m = ray.origin - sphere.center;
        float b = glm::dot(m, ray.direction);
        float c = glm::dot(m, m) - sphere.radius * sphere.radius;
        if (c > 0.0f && b > 0.0f)
        {
            return false;
        }
        float discr = b * b - c;
        if (discr < 0.0f)
        {
            return false;
        }
        return true;
    }
    return false;
}

bool Math::Intersects(const Sphere& sphere, const Ray& ray)
{
    return Intersects(ray, sphere);
}

bool Math::Intersects(const Line& a, const Line& b)
{
    glm::vec3 d1 = a.v2 - a.v1;
    glm::vec3 d2 = b.v2 - b.v1;
    glm::vec3 r = a.v1 - b.v1;
    float a1 = glm::dot(d1, d1);
    float a2 = glm::dot(d1, d2);
    float b1 = glm::dot(d1, d2);
    float b2 = glm::dot(d2, d2);
    float c1 = glm::dot(d1, r);
    float c2 = glm::dot(d2, r);
    float det = a1 * b2 - a2 * b1;

    if (det < 1e-4f)
    {
        return false;
    }
    
    float s = (b2 * c1 - b1 * c2) / det;
    float t = (a1 * c2 - a2 * c1) / det;

    return s >= 0.0f && s <= 1.0f && t >= 0.0f && t <= 1.0f;
}