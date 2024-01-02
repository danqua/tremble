#pragma once
#include <glm/glm.hpp>

namespace Math
{

struct Sphere
{
    // The center of the sphere.
    glm::vec3 center;

    // The radius of the sphere.
    float radius;

    // Creates an uninitialized new sphere.
    Sphere();

    // Creates and initializes a new sphere from the given center and radius.
    Sphere(const glm::vec3& center, float radius);

    // Creates a sphere that encloses the given points.
    Sphere(const glm::vec3* points, size_t count);

    // Extends the sphere to include the given point.
    Sphere& operator+=(const glm::vec3& point);

    // Returns true if the sphere contains the given point.
    bool ContainsPoint(const glm::vec3& point) const;

    // Returns true if the sphere contains the given sphere.
    bool ContainsSphere(const Sphere& sphere) const;
};

}