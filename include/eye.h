#pragma once

#include <glm/vec3.hpp>

namespace renderer
{
    struct Eye
    {
        glm::vec3 position;
        glm::vec3 target;
        glm::vec3 up;
    };
}
