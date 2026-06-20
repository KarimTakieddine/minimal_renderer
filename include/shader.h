#pragma once

namespace renderer
{
    struct Shader
    {
        enum class Type : unsigned
        {
            UNKNOWN     = 0,
            VERTEX      = 1,
            FRAGMENT    = 2
        };

        Type type           { Type::UNKNOWN };
        unsigned int index  { 0 };
    };
}
