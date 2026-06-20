#pragma once

#include <cstddef>
#include <cstdint>

#include <bump_allocator.hpp>

namespace renderer
{
    using Allocator = BumpAllocator<16>;

    void allocate(Allocator* allocator);
    void render(const Allocator* allocator);
}
