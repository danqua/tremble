#include "Line2.h"

Line2::Line2()
    : m_v1(0.0f, 0.0f)
    , m_v2(0.0f, 0.0f)
{
}

Line2::Line2(const glm::vec2& v1, const glm::vec2& v2)
    : m_v1(v1)
    , m_v2(v2)
{
}

Line2::Line2(const Line2& other)
    : m_v1(other.m_v1)
    , m_v2(other.m_v2)
{
}

glm::vec2 Line2::GetNormal() const
{
    glm::vec2 normal = glm::normalize(m_v2 - m_v1);
    return glm::vec2(-normal.y, normal.x);
}