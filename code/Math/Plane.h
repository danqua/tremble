#pragma once
#include <glm/glm.hpp>

struct Plane
{
    // The normal of the plane.
    glm::vec3 normal;

    // The distance of the plane from the origin.
    float distance;

    // Creates an uninitialized new plane.
    Plane();

    // Creates and initializes a new plane from the given normal and distance.
    Plane(const glm::vec3& normal, float distance);

    // Creates and initializes a new plane from the given points.
    Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    // Returns the distance of the given point from the plane.
    float GetClosestDistanceToPoint(const glm::vec3& point) const;

    // Returns the side of the plane on which the given point lies.
    int GetPointSide(const glm::vec3& point) const;
};