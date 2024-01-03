#include "Line.h"

Line::Line()
    : v1(std::numeric_limits<float>::quiet_NaN())
    , v2(std::numeric_limits<float>::quiet_NaN())
{
}

Line::Line(const glm::vec3& v1, const glm::vec3& v2)
    : v1(v1)
    , v2(v2)
{
}

float Line::GetClosestDistanceToPoint(const glm::vec3& point) const
{
    glm::vec3 v = v2 - v1;
    glm::vec3 w = point - v1;

    float c1 = glm::dot(w, v);
    if (c1 <= 0.0f)
    {
        return glm::distance(point, v1);
    }

    float c2 = glm::dot(v, v);
    if (c2 <= c1)
    {
        return glm::distance(point, v2);
    }

    float b = c1 / c2;
    glm::vec3 pb = v1 + v * b;
    return glm::distance(point, pb);
}

glm::vec3 Line::GetClosestPoint(const glm::vec3& point) const
{
    glm::vec3 v = v2 - v1;
    glm::vec3 w = point - v1;

    float c1 = glm::dot(w, v);
    if (c1 <= 0.0f)
    {
        return v1;
    }

    float c2 = glm::dot(v, v);
    if (c2 <= c1)
    {
        return v2;
    }

    float b = c1 / c2;
    return v1 + v * b;
}

bool Line::ContainsPoint(const glm::vec3& point) const
{
    return glm::distance(point, GetClosestPoint(point)) < 1e-4f;
}

bool Line::ContainsLine(const Line& line) const
{
    return ContainsPoint(line.v1) && ContainsPoint(line.v2);
}