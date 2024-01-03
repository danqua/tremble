#include "Plane.h"

Plane::Plane()
    : normal(0.0f, 0.0f, 0.0f)
    , distance(0.0f)
{
}

Plane::Plane(const glm::vec3& normal, float distance)
    : normal(normal)
    , distance(distance)
{
}

Plane::Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    normal = glm::normalize(glm::cross(b - a, c - a));
    distance = -glm::dot(normal, a);
}

float Plane::GetClosestDistanceToPoint(const glm::vec3& point) const
{
    return glm::dot(normal, point) + distance;
}

int Plane::GetPointSide(const glm::vec3& point) const
{
    float distance = GetClosestDistanceToPoint(point);

    if (distance > 0.0f)
    {
        return 1;
    }
    else if (distance < 0.0f)
    {
        return -1;
    }
    return 0;
}