#include "Plane2D.h"

Plane2D::Plane2D()
{
    m_normal = glm::vec2(0.0f, 0.0f);
    m_distance = 0.0f;
}

Plane2D::Plane2D(const glm::vec2& normal, float distance)
{
    m_normal = normal;
    m_distance = distance;
}

Plane2D::Plane2D(const glm::vec2& normal, const glm::vec2& point)
{
    m_normal = normal;
    m_distance = glm::dot(normal, point);
}

Plane2D::Plane2D(const Plane2D& other)
{
    m_normal = other.m_normal;
    m_distance = other.m_distance;
}

int Plane2D::GetSide(const glm::vec2& point) const
{
    float dot = glm::dot(m_normal, point) - m_distance;
    if (dot > 0.0f)
    {
        return 1;
    }
    else if (dot < 0.0f)
    {
        return -1;
    }
    return 0;
}

glm::vec2 Plane2D::GetNormal() const
{
    return m_normal;
}

float Plane2D::GetDistance() const
{
    return m_distance;
}
