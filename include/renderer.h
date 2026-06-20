#pragma once

#include <cstddef>
#include <cstdint>

#include <bump_allocator.hpp>

#include "mesh.hpp"

namespace renderer
{
    using Allocator = BumpAllocator<16>;

    void allocateMeshes(Allocator* allocator, size_t count, const ConstMesh* meshes);
    void uploadMeshes(const Allocator* allocator);
    void uploadMesh(const ConstMesh* mesh);
    void allocate(Allocator* allocator);
    void render(const Allocator* allocator);
}
