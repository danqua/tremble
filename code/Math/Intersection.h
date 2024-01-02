#pragma once

#include "Box.h"
#include "Ray.h"
#include "Line.h"
#include "Sphere.h"

namespace Math
{

// Returns true if the given boxes intersect.
bool Intersects(const Box& a, const Box& b);

// Returns true if the given spheres intersect.
bool Intersects(const Sphere& a, const Sphere& b);

// Returns true if the given box and sphere intersect.
bool Intersects(const Box& a, const Sphere& b);

// Returns true if the given sphere and box intersect.
bool Intersects(const Sphere& a, const Box& b);

// Returns true if the given ray and box intersect.
bool Intersects(const Ray& ray, const Box& box);

// Returns true if the given box and ray intersect.
bool Intersects(const Box& box, const Ray& ray);

// Returns true if the given ray and sphere intersect.
bool Intersects(const Ray& ray, const Sphere& sphere);

// Returns true if the given sphere and ray intersect.
bool Intersects(const Sphere& sphere, const Ray& ray);

// Returns true if the given lines are intersecting.
bool Intersects(const Line& a, const Line& b);

}