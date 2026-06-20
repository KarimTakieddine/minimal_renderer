#pragma once

#include <cstddef>
#include <cstdint>

#include <bump_allocator.hpp>

#include "locations_descriptor.h"
#include "mesh.hpp"
#include "shader.h"

namespace renderer
{
    static constexpr size_t ALLOCATOR_SIZE      = 1 << 13;
    static constexpr uint64_t MEMORY_ALIGNMENT  = 16;

    using Allocator = BumpAllocator<16>;

    uint64_t getBufferOffset(const Allocator* allocator);
    uint64_t getVertexArrayOffset(const Allocator* allocator);
    uint64_t getTextureOffset(const Allocator* allocator);
    uint64_t getShaderOffset(const Allocator* allocator);
    uint64_t getShaderProgramOffset(const Allocator* allocator);
    // TODO(Karim):
    uint64_t getLocationsDescriptorOffset(const Allocator* allocator);
    uint64_t getMeshDataOffset(const Allocator* allocator);

    void allocateBuffers(Allocator* allocator, size_t count);
    void generateBuffers(Allocator* allocator);
    void freeBuffers(Allocator* allocator);

    void allocateVertexArrays(Allocator* allocator, size_t count);
    void generateVertexArrays(Allocator* allocator);
    void freeVertexArrays(Allocator* allocator);

    void allocateTextures(Allocator* allocator, size_t count);
    void generateTextures(Allocator* allocator);
    void freeTextures(Allocator* allocator);

    void allocateMeshes(Allocator* allocator, size_t count, const ConstMesh* meshes);
    void generateMeshes(Allocator* allocator);
    void uploadMeshes(const Allocator* allocator);
    void uploadMesh(const ConstMesh* mesh);

    void allocateShaders(Allocator* allocator, size_t count);
    void generateShaders(Allocator* allocator, size_t count, Shader* shaders);
    void allocateShaderPrograms(Allocator* allocator, size_t count);
    void generateShaderPrograms(Allocator* allocator);
    void compileShaderProgram(const Allocator* allocator, size_t index, size_t shaderCount, const size_t* shaderIndices);
    void freeShaders(Allocator* allocator);

    // TODO(Karim):

    void allocateLocationsDescriptor(Allocator* allocator);
    void setShaderLocations(unsigned int program, LocationsDescriptor* descriptor);

    void allocate(Allocator* allocator);
    void initializeGraphicsResources(Allocator* allocator);
    void freeGraphicsResources(Allocator* allocator);
    void initializeGraphicsState();
    void clearFrameBuffer();

    void render(const Allocator* allocator);
}
