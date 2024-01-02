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

    // Creates and initializes a new plane from the given points.
    Plane(const glm::vec3* points, size_t count);

    // Returns the distance of the given point from the plane.
    float GetClosestDistanceToPoint(const glm::vec3& point) const;

    /*
    // Returns true if the given point is in front of the plane.
    bool IsInFront(const glm::vec3& point) const;

    // Returns true if the given point is behind the plane.
    bool IsBehind(const glm::vec3& point) const;

    // Returns true if the given point is on the plane.
    bool IsOn(const glm::vec3& point) const;

    // Returns true if the given box is in front of the plane.
    bool IsInFront(const Box& box) const;

    // Returns true if the given box is behind the plane.
    bool IsBehind(const Box& box) const;

    // Returns true if the given box intersects the plane.
    bool Intersects(const Box& box) const;

    // Returns true if the given sphere is in front of the plane.
    bool IsInFront(const Sphere& sphere) const;

    // Returns true if the given sphere is behind the plane.
    bool IsBehind(const Sphere& sphere) const;

    // Returns true if the given sphere intersects the plane.
    bool Intersects(const Sphere& sphere) const;

    // Returns true if the given ray intersects the plane.
    bool Intersects(const Ray& ray) const;

    // Returns true if the given frustum intersects the plane.
    bool Intersects(const Frustum& frustum) const;
    // Returns true if the given plane intersects the plane.
    bool Intersects(const Plane& plane) const;

    // Returns true if the given plane is parallel to the plane.
    bool IsParallel(const Plane& plane) const;

    */
    // Returns true if
};