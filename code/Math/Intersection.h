#pragma once

#include "Box.h"
#include "Ray.h"
#include "Line.h"
#include "Sphere.h"
#include "Frustum.h"

namespace Math
{
    // Returns true if the given boxes intersect.
    bool Intersects(const Box& a, const Box& b);

    // Returns true if the given spheres intersect.
    bool Intersects(const Sphere& a, const Sphere& b);

    // Returns true if the given box and sphere intersect.
    bool Intersects(const Box& a, const Sphere& b);

    // Returns true if the given ray and box intersect.
    bool Intersects(const Ray& ray, const Box& box);

    // Returns true if the given ray and sphere intersect.
    bool Intersects(const Ray& ray, const Sphere& sphere);

    // Returns true if the given lines are intersecting.
    bool Intersects(const Line& a, const Line& b);

    // Returns true if the given line and sphere intersect.
    bool Intersects(const Line& line, const Sphere& sphere);

    // Returns true if the given sphere and line intersect.
    bool Intersects(const Sphere& sphere, const Line& line);

    // Returns true if the given line and box intersect.
    bool Intersects(const Line& line, const Box& box);

    // Returns true if the given box and line intersect.
    bool Intersects(const Box& box, const Line& line);

    // Returns true if the given line and plane intersect.
    bool Intersects(const Line& line, const Plane& plane);

    // Returns true if the given plane and line intersect.
    bool Intersects(const Plane& plane, const Line& line);

    // Returns true if the given line and frustum intersect.
    bool Intersects(const Line& line, const Frustum& frustum);

    // Returns true if the given frustum and line intersect.
    bool Intersects(const Frustum& frustum, const Line& line);
}