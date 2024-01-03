#include "Frustum.h"

Frustum::Frustum()
{
}

Frustum::Frustum(const glm::mat4& projectionViewMatrix)
{
    ExtractFrom(projectionViewMatrix);
}

void Frustum::ExtractFrom(const glm::mat4& projectionViewMatrix)
{
    // Left clipping plane
    planes[0].normal.x = projectionViewMatrix[0][3] + projectionViewMatrix[0][0];
    planes[0].normal.y = projectionViewMatrix[1][3] + projectionViewMatrix[1][0];
    planes[0].normal.z = projectionViewMatrix[2][3] + projectionViewMatrix[2][0];
    planes[0].distance = projectionViewMatrix[3][3] + projectionViewMatrix[3][0];

    // Right clipping plane
    planes[1].normal.x = projectionViewMatrix[0][3] - projectionViewMatrix[0][0];
    planes[1].normal.y = projectionViewMatrix[1][3] - projectionViewMatrix[1][0];
    planes[1].normal.z = projectionViewMatrix[2][3] - projectionViewMatrix[2][0];
    planes[1].distance = projectionViewMatrix[3][3] - projectionViewMatrix[3][0];

    // Top clipping plane
    planes[2].normal.x = projectionViewMatrix[0][3] - projectionViewMatrix[0][1];
    planes[2].normal.y = projectionViewMatrix[1][3] - projectionViewMatrix[1][1];
    planes[2].normal.z = projectionViewMatrix[2][3] - projectionViewMatrix[2][1];
    planes[2].distance = projectionViewMatrix[3][3] - projectionViewMatrix[3][1];

    // Bottom clipping plane
    planes[3].normal.x = projectionViewMatrix[0][3] + projectionViewMatrix[0][1];
    planes[3].normal.y = projectionViewMatrix[1][3] + projectionViewMatrix[1][1];
    planes[3].normal.z = projectionViewMatrix[2][3] + projectionViewMatrix[2][1];
    planes[3].distance = projectionViewMatrix[3][3] + projectionViewMatrix[3][1];

    // Near clipping plane
    planes[4].normal.x = projectionViewMatrix[0][3] + projectionViewMatrix[0][2];
    planes[4].normal.y = projectionViewMatrix[1][3] + projectionViewMatrix[1][2];
    planes[4].normal.z = projectionViewMatrix[2][3] + projectionViewMatrix[2][2];
    planes[4].distance = projectionViewMatrix[3][3] + projectionViewMatrix[3][2];
}

bool Frustum::ContainsPoint(const glm::vec3& point) const
{
    for (int i = 0; i < 6; ++i)
    {
        if (planes[i].GetClosestDistanceToPoint(point) < 0.0f)
        {
            return false;
        }
    }

    return true;
}

bool Frustum::ContainsSphere(const Sphere& sphere) const
{
    for (int i = 0; i < 6; ++i)
    {
        if (planes[i].GetClosestDistanceToPoint(sphere.center) < -sphere.radius)
        {
            return false;
        }
    }

    return true;
}

bool Frustum::ContainsBox(const Box& box) const
{
    std::array<glm::vec3, 8> cornerVertices;
    box.GetCornerPoints(cornerVertices);

    for (int i = 0; i < 8; ++i)
    {
        if (ContainsPoint(cornerVertices[i]))
        {
            return false;
        }
    }

    return true;
}

bool Frustum::IntersectsBox(const Box& box) const
{
    std::array<glm::vec3, 8> cornerVertices;
    box.GetCornerPoints(cornerVertices);

    for (int i = 0; i < 6; ++i)
    {
        bool inside = false;

        for (int j = 0; j < 8; ++j)
        {
            if (planes[i].GetClosestDistanceToPoint(cornerVertices[j]) >= 0.0f)
            {
                inside = true;
                break;
            }
        }

        if (!inside)
        {
            return false;
        }
    }

    return true;
}