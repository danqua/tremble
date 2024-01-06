#pragma once
#include <glm/glm.hpp>

class Plane2D
{
public:
    Plane2D();
    Plane2D(const glm::vec2& normal, float distance);
    Plane2D(const glm::vec2& normal, const glm::vec2& point);
    Plane2D(const Plane2D& other);

    int GetSide(const glm::vec2& point) const;


    glm::vec2 GetNormal() const;
    float GetDistance() const;

private:
    glm::vec2 m_normal;
    float m_distance;
};