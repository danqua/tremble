#pragma once
#include <array>
#include <glm/glm.hpp>

struct Box
{
    // The minimum and maximum extents of the box.
    glm::vec3 min;

    // The maximum extents of the box.
    glm::vec3 max;

    // Whether or not the box is valid.
    bool isValid;

    // Creates a new box with uninitialized extents and marks it as invalid.
    Box();

    // Creates and initializes a new box from the given extents.
    Box(const glm::vec3& min, const glm::vec3& max);

    // Creates and initializes a new box from center and half extent.
    Box(const glm::vec3& center, float halfExtent);

    // Creates and initializes a new box from the given points.
    Box(const glm::vec3* points, size_t count);

    // Extends the box to include the given box.
    Box& operator+=(const Box& box);

    // Extends the box to include the given point.
    Box& operator+=(const glm::vec3& point);

    // Scales the box by the given scalar.
    Box& operator*=(float scalar);

    // Expands the box by the given amount.
    Box& Expand(const glm::vec3& amount);

    // Returns the center of the box.
    glm::vec3 GetCenter() const;

    // Returns the size of the box.
    glm::vec3 GetSize() const;

    // Returns the extents of the box.
    glm::vec3 GetExtents() const;

    // Returns the volume of the box.
    float GetVolume() const;

    // Returns true if the box contains the given point.
    bool ContainsPoint(const glm::vec3& point) const;

    // Returns true if the box contains the given box.
    bool ContainsBox(const Box& box) const;

    // Returns the box's corners.
    void GetCornerPoints(std::array<glm::vec3, 8>& points) const;
};