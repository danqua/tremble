#include "Sphere.h"

Sphere::Sphere()
    : center(std::numeric_limits<float>::quiet_NaN())
    , radius(std::numeric_limits<float>::quiet_NaN())
{
}

Sphere::Sphere(const glm::vec3& center, float radius)
    : center(center)
    , radius(radius)
{
}

Sphere::Sphere(const glm::vec3* points, size_t count)
    : center(std::numeric_limits<float>::quiet_NaN())
    , radius(std::numeric_limits<float>::quiet_NaN())
{
    for (size_t i = 0; i < count; ++i)
    {
        *this += points[i];
    }
}

Sphere& Sphere::operator+=(const glm::vec3& point)
{
    if (std::isnan(radius))
    {
        center = point;
        radius = 0.0f;
    }
    else
    {
        float distance = glm::distance(point, center);
        if (distance > radius)
        {
            radius = (radius + distance) * 0.5f;
            center += (point - center) * ((radius - distance) / distance);
        }
    }
    return *this;
}

bool Sphere::ContainsPoint(const glm::vec3& point) const
{
    return glm::distance(point, center) <= radius;
}

bool Sphere::ContainsSphere(const Sphere& sphere) const
{
    return glm::distance(sphere.center, center) + sphere.radius <= radius;
}