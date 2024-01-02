#pragma once
#include <glm/glm.hpp>

namespace Math
{

struct Ray
{
    // The origin of the ray.
    glm::vec3 origin;

    // The direction of the ray.
    glm::vec3 direction;

    // Creates an uninitialized new ray.
    Ray();

    // Creates and initializes a new ray from the given origin and direction.
    Ray(const glm::vec3& origin, const glm::vec3& direction);

    // Returns the point at the given distance along the ray.
    glm::vec3 GetPointAtDistance(float distance) const;

    // Returns the closest distance of the given point from the ray.
    float GetClosestDistanceToPoint(const glm::vec3& point) const;
};

}