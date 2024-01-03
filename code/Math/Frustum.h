#pragma once
#include "Box.h"
#include "Plane.h"
#include "Sphere.h"

struct Frustum
{
    // The six planes of the frustum.
    Plane planes[6];

    // Creates an uninitialized new frustum.
    Frustum();

    // Creates and initializes a new frustum from the given projection-view matrix.
    Frustum(const glm::mat4& projectionViewMatrix);

    // Creates and initializes a new frustum from the given projection matrix and view matrix.
    void ExtractFrom(const glm::mat4& projectionViewMatrix);

    // Returns true if the given point is inside the frustum.
    bool ContainsPoint(const glm::vec3& point) const;

    // Returns true if the given sphere is inside the frustum.
    bool ContainsSphere(const Sphere& sphere) const;

    // Returns true if the given box is inside the frustum.
    bool ContainsBox(const Box& box) const;

    // Returnns true if the given box intersects the frustum.
    bool IntersectsBox(const Box& box) const;
};