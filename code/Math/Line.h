#pragma once
#include <glm/glm.hpp>

struct Line
{
    // The start of the line.
    glm::vec3 v1;

    // The end of the line.
    glm::vec3 v2;

    // Creates an uninitialized new line.
    Line();

    // Creates and initializes a new line from the given start and end.
    Line(const glm::vec3& v1, const glm::vec3& v2);

    // Returns the closest distance of the given point from the line.
    float GetClosestDistanceToPoint(const glm::vec3& point) const;

    // Returns the closest point on the line to the given point.
    glm::vec3 GetClosestPoint(const glm::vec3& point) const;

    // Returns true if the point is on the line.
    bool ContainsPoint(const glm::vec3& point) const;

    // Returns true if the given line is on the line.
    bool ContainsLine(const Line& line) const;
};