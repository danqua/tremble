#pragma once
#include <glm/glm.hpp>

class Line2
{
public:
    Line2();
    Line2(const glm::vec2& v1, const glm::vec2& v2);
    Line2(const Line2& other);

    glm::vec2 GetNormal() const;

private:
    glm::vec2 m_v1;
    glm::vec2 m_v2;
};