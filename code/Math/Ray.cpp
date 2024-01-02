#include "Ray.h"

using namespace Math;

Ray::Ray()
    : origin(std::numeric_limits<float>::quiet_NaN())
    , direction(std::numeric_limits<float>::quiet_NaN())
{
}

Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
    : origin(origin)
    , direction(direction)
{
}

glm::vec3 Ray::GetPointAtDistance(float distance) const
{
    return origin + direction * distance;
}

float Ray::GetClosestDistanceToPoint(const glm::vec3& point) const
{
    return glm::length(glm::cross(direction, point - origin));
}