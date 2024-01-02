#include "Box.h"
#include <glm/gtx/component_wise.hpp>

using namespace Math;

Box::Box()
    : min(std::numeric_limits<float>::quiet_NaN())
    , max(std::numeric_limits<float>::quiet_NaN())
    , isValid(false)
{
}

Box::Box(const glm::vec3& min, const glm::vec3& max)
    : min(min)
    , max(max)
    , isValid(true)
{
}

Box::Box(const glm::vec3& center, float halfExtent)
    : min(center - halfExtent)
    , max(center + halfExtent)
    , isValid(true)
{
}

Box::Box(const glm::vec3* points, size_t count)
    : min(std::numeric_limits<float>::max())
    , max(std::numeric_limits<float>::lowest())
    , isValid(false)
{
    for (size_t i = 0; i < count; ++i)
    {
        *this += points[i];
    }
}

Box& Box::operator+=(const Box& box)
{
    if (box.isValid)
    {
        *this += box.min;
        *this += box.max;
    }
    return *this;
}

Box& Box::operator+=(const glm::vec3& point)
{
    if (isValid)
    {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }
    else
    {
        min = point;
        max = point;
        isValid = true;
    }
    return *this;
}

glm::vec3 Box::GetCenter() const
{
    return (min + max) * 0.5f;
}

glm::vec3 Box::GetSize() const
{
    return max - min;
}

glm::vec3 Box::GetExtents() const
{
    return GetSize() * 0.5f;
}

float Box::GetVolume() const
{
    glm::vec3 size = GetSize();
    return size.x * size.y * size.z;
}

bool Box::ContainsPoint(const glm::vec3& point) const
{
    return isValid && glm::all(glm::greaterThanEqual(point, min)) && glm::all(glm::lessThanEqual(point, max));
}

bool Box::ContainsBox(const Box& box) const
{
    return isValid && box.isValid && glm::all(glm::greaterThanEqual(box.min, min)) && glm::all(glm::lessThanEqual(box.max, max));
}

void Box::GetCornerPoints(std::array<glm::vec3, 8>& points) const
{
    points[0] = glm::vec3(min.x, min.y, min.z);
    points[1] = glm::vec3(max.x, min.y, min.z);
    points[2] = glm::vec3(min.x, max.y, min.z);
    points[3] = glm::vec3(max.x, max.y, min.z);
    points[4] = glm::vec3(min.x, min.y, max.z);
    points[5] = glm::vec3(max.x, min.y, max.z);
    points[6] = glm::vec3(min.x, max.y, max.z);
    points[7] = glm::vec3(max.x, max.y, max.z);
}
