#pragma once

#include <cstddef>
#include <cstdint>

#include <bump_allocator.hpp>

#include "camera.h"
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
    uint64_t getMeshDataOffset(const Allocator* allocator);
    uint64_t getLocationsDescriptorOffset(const Allocator* allocator);
    uint64_t getCameraEyeOffset(const Allocator* allocator);
    uint64_t getCameraFrustumOffset(const Allocator* allocator);
    uint64_t getCameraOffset(const Allocator* allocator);
    uint64_t getUniformBufferOffset(const Allocator* allocator);
    uint64_t getUniformSegmentOffset(const Allocator* allocator);

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

    void allocateLocationsDescriptors(Allocator* allocator, size_t count);
    bool setShaderLocations(Allocator* allocator, size_t programIndex, size_t descriptorIndex);

    void allocateCamera(Allocator* allocator);
    void setCameraEye(Allocator* allocator, const Camera::Eye* eye);
    void setCameraFrustum(Allocator* allocator, const Camera::Frustum* frustum);
    void updateCamera(Allocator* allocator);

    void allocateUniformBuffer(Allocator* allocator, size_t segmentCount);
    bool generateUniformBuffer(Allocator* allocator, size_t programIndex, const char* name, const char* const* names);
    bool mapCameraUniforms(Allocator* allocator);

    void allocate(Allocator* allocator);
    void initializeGraphicsResources(Allocator* allocator);
    void freeGraphicsResources(Allocator* allocator);
    void initializeGraphicsState();
    void clearFrameBuffer();

    void uploadUniformBuffer(const Allocator* allocator);
    void render(const Allocator* allocator);
}
