#pragma once

#include <cstddef>
#include <cstdint>

#include <bump_allocator.hpp>

namespace renderer
{
    struct Mesh;

    using Allocator = BumpAllocator<16>;

    void allocateMeshes(Allocator* allocator, size_t count, const Mesh* meshes);
    void allocate(Allocator* allocator);
    void render(const Allocator* allocator);
}
