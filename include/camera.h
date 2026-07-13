#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

namespace renderer
{
    struct Camera
    {
        struct Eye
        {
            glm::vec3 position;
            glm::vec3 target;
            glm::vec3 up;
        };

        struct Frustum
        {
            float fov	{ 0.0f };
            float aspect{ 0.0f };
            float near	{ 0.0f };
            float far	{ 0.0f };
        };

        glm::mat4 localToWorld	{ 1.0f };
        glm::mat4 localRotation	{ 1.0f };
        glm::mat4 view			{ 1.0f };
        glm::mat4 projection	{ 1.0f };
    };
}
