#pragma once

#include <cstddef>
#include <cstdint>

#include <bump_allocator.hpp>

#include "mesh.hpp"

namespace renderer
{
    static constexpr size_t ALLOCATOR_SIZE      = 1 << 13;
    static constexpr uint64_t MEMORY_ALIGNMENT  = 16;

    using Allocator = BumpAllocator<16>;

    uint64_t getBufferOffset(const Allocator* allocator);
    uint64_t getVertexArrayOffset(const Allocator* allocator);
    uint64_t getTextureOffset(const Allocator* allocator);
    uint64_t getMeshDataOffset(const Allocator* allocator);

    void allocateBuffers(Allocator* allocator, size_t count);
    void freeBuffers(Allocator* allocator);

    void allocateVertexArrays(Allocator* allocator, size_t count);
    void freeVertexArrays(Allocator* allocator);

    void allocateTextures(Allocator* allocator, size_t count);
    void freeTextures(Allocator* allocator);

    void generateMeshes(Allocator* allocator, size_t count, ConstMesh* meshes);
    void allocateMeshes(Allocator* allocator, size_t count, const ConstMesh* meshes);
    void uploadMeshes(const Allocator* allocator);
    void uploadMesh(const ConstMesh* mesh);
    void allocate(Allocator* allocator);
    void render(const Allocator* allocator);
}
