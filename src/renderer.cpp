#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "eye.h"
#include "frustum.h"
#include "locations_descriptor.h"
#include "mesh.hpp"
#include "render_batch.h"
#include "render_entity.h"
#include "renderer.hpp"
#include "shader.h"
#include "uniform_buffer_segment.h"

namespace renderer
{
    uint64_t getBufferOffset(const Allocator* allocator)
    {
        return 0;
    }

    uint64_t getVertexArrayOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor;

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(
            static_cast<const std::byte*>(allocator->peek()),
            ALLOCATOR_SIZE);
        
        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto bufferCount = allocatorMemoryView.read_object<uint64_t>(getBufferOffset(allocator));
        cursor.step_array<unsigned int>(*bufferCount.data());

        return cursor.getOffset();
    }

    uint64_t getTextureOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getVertexArrayOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(
            static_cast<const std::byte*>(allocator->peek()),
            ALLOCATOR_SIZE);
        
        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto vertexArrayCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
        cursor.step_array<unsigned int>(*vertexArrayCount.data());

        return cursor.getOffset();
    }

    uint64_t getShaderOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getTextureOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);
     
        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto textureCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
        cursor.step_array<unsigned int>(*textureCount.data());

        return cursor.getOffset();
    }

    uint64_t getShaderProgramOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getShaderOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);
     
        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto shaderCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
        cursor.step_array<Shader>(*shaderCount.data());

        return cursor.getOffset();
    }

    uint64_t getMeshDataOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getShaderProgramOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);
     
        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto shaderProgramCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
        cursor.step_array<unsigned int>(*shaderProgramCount.data());

        return cursor.getOffset();
    }

    uint64_t getLocationsDescriptorOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getMeshDataOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto meshCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
        cursor.step<uint64_t>();

        for (uint64_t i = 0; i < *meshCount.data(); ++i)
        {
            cursor.step<MeshBufferIndices>();

            const auto vertexCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
            cursor.step_array<Vertex>(*vertexCount.data());

            const auto triangleCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
            cursor.step_array<unsigned int>(*triangleCount.data());
        }

        return cursor.getOffset();
    }

    uint64_t getCameraEyeOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getLocationsDescriptorOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto locationsDescriptorCount = allocatorMemoryView.read_object<uint64_t>(cursor.getOffset());
        cursor.step_array<LocationsDescriptor>(*locationsDescriptorCount.data());

        return cursor.getOffset();
    }

    uint64_t getCameraFrustumOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getCameraEyeOffset(allocator));
        cursor.step<Eye>();
        return cursor.getOffset();
    }

    uint64_t getCameraOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getCameraFrustumOffset(allocator));
        cursor.step<Frustum>();
        return cursor.getOffset();
    }

    uint64_t getUniformBufferOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getCameraOffset(allocator));
        cursor.step<Camera>();
        return cursor.getOffset();
    }

    uint64_t getUniformSegmentOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getUniformBufferOffset(allocator));
        cursor.step<unsigned int>();
        return cursor.getOffset();
    }

    uint64_t getRenderBatchOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getUniformSegmentOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        ConstMemoryView allocatorMemoryView(allocatorSpan);
        const auto uniformSegments = allocatorMemoryView.read_contiguous_array<UniformBufferSegment>(cursor.getOffset());
        cursor.step_array<UniformBufferSegment>(uniformSegments.size());

        return cursor.getOffset();
    }

    void allocateBuffers(Allocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(count);
        allocator->requestMemoryArray<unsigned int>(count);
    }

    void allocateVertexArrays(Allocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(count);
        allocator->requestMemoryArray<unsigned int>(count);
    }

    void allocateTextures(Allocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(count);
        allocator->requestMemoryArray<unsigned int>(count);
    }

    void allocateMeshes(Allocator* allocator, size_t count, const Mesh* meshes)
    {
        auto* meshCount = allocator->requestMemory<uint64_t>(count);
        
        for (size_t i = 0; i < *meshCount; ++i)
        {
            const Mesh& mesh = meshes[i];

            auto* bufferIndices = allocator->requestMemory<MeshBufferIndices>();

            auto* vertexCount   = allocator->requestMemory<uint64_t>(mesh.vertexCount);
            auto* vertices      = allocator->requestMemoryArray<Vertex>(*vertexCount);
            for (uint64_t j = 0; j < *vertexCount; ++j)
            {
                vertices[j] = mesh.vertices[j];
            }

            auto* triangleCount = allocator->requestMemory<uint64_t>(mesh.triangleCount);
            auto* triangles     = allocator->requestMemoryArray<unsigned int>(*triangleCount);
            for (uint64_t j = 0; j < *triangleCount; ++j)
            {
                triangles[j] = mesh.triangles[j]; 
            }
        }
    }

    void allocateShaders(Allocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(count);
        allocator->requestMemoryArray<unsigned int>(count);
    }

    void allocateShaderPrograms(Allocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(count);
        allocator->requestMemoryArray<unsigned int>(count);
    }

    void allocateLocationsDescriptors(Allocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(count);
        allocator->requestMemoryArray<LocationsDescriptor>(count);
    }

    void allocateCamera(Allocator* allocator)
    {
        allocator->requestMemory<Eye>();
        allocator->requestMemory<Frustum>();
        allocator->requestMemory<Camera>();
    }

    void setCameraEye(const MutableGraphicsMemory& memory, const Eye* eye)
    {
        *( memory.cameraEye.data() ) = *eye;
    }

    void setCameraFrustum(const MutableGraphicsMemory& memory, const Frustum* frustum)
    {
        *( memory.cameraFrustum.data() ) = *frustum;
    }

    void updateCamera(const MutableGraphicsMemory& memory)
    {
        const auto* eye     = memory.cameraEye.data();
        const auto* frustum = memory.cameraFrustum.data();
        auto* camera        = memory.camera.data();

        camera->projection  = glm::perspective(glm::radians(frustum->fov), frustum->aspect, frustum->near, frustum->far);
        camera->view        = glm::lookAtRH(eye->position, eye->target, eye->up);
    }

    void allocateUniformBuffer(Allocator* allocator, size_t segmentCount)
    {
        allocator->requestMemory<unsigned int>();
        allocator->requestMemory<uint64_t>(segmentCount);
        allocator->requestMemoryArray<UniformBufferSegment>(segmentCount);
    }

    bool mapCameraUniforms(const MutableGraphicsMemory& memory)
    {
        const auto segments = memory.uniformBufferSegments;

        if (segments.size() != 4)
        {
            return false;
        }

        auto* segmentData = segments.data();

        const auto* camera = memory.camera.data();

        segmentData[0].data = &camera->projection;
        segmentData[1].data = &camera->localToWorld;
        segmentData[2].data = &camera->localRotation;
        segmentData[3].data = &camera->view;

        return true;
    }

    void allocateRenderBatches(Allocator* allocator, size_t count, const size_t* entityCounts)
    {
        allocator->requestMemory<uint64_t>(count);

        for (size_t i = 0; i < count; ++i)
        {
            allocator->requestMemory<RenderBatch>();

            const size_t entityCount = entityCounts[i];
            allocator->requestMemory<uint64_t>(entityCount);
            allocator->requestMemoryArray<RenderEntity>(entityCount);
        }
    }

    void allocate(Allocator* allocator)
    {
        allocator->allocate(ALLOCATOR_SIZE);

        allocateBuffers(allocator, 3);
        allocateVertexArrays(allocator, 1);
        allocateTextures(allocator, 1);
        allocateShaders(allocator, 2);
        allocateShaderPrograms(allocator, 1);

        Vertex quadVertices[4]          = {
            { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
            { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        };
        unsigned int quadTriangles[6]   = { 0, 2, 1, 0, 3, 2 };
        Mesh meshes[1] = { { quadVertices, quadTriangles, 4, 6 } };
        allocateMeshes(allocator, 1, meshes);
        allocateLocationsDescriptors(allocator, 1);
        allocateCamera(allocator);
        allocateUniformBuffer(allocator, 4);

        const size_t entityCount = 4;
        allocateRenderBatches(allocator, 1, &entityCount);
    }

    void initializeGraphicsResources(const MutableGraphicsMemory& memory)
    {
        generateBuffers(memory);
        generateVertexArrays(memory);
        generateTextures(memory);

        Shader shaderList[2] = {
            { "./shaders/3d_transform_vertex.slh", Shader::Type::VERTEX },
            { "./shaders/3d_transform_fragment.slh", Shader::Type::FRAGMENT },
        };
        generateShaders(memory, 2, shaderList);

        size_t shaderIndices[2] = { 0, 1 };
        generateShaderPrograms(memory);
        compileShaderProgram(memory, 0, 2, shaderIndices);

        generateMeshes(memory);

        uploadMeshes(freezeGraphicsMemory(memory));

        setShaderLocations(memory, 0, 0);

        Eye cameraEye = {
            .position   = glm::vec3{ 0.0f, 0.0f, 10.0f },
            .target	    = glm::vec3{ 0.0f, 0.0f, 0.0f },
            .up		    = glm::vec3{ 0.0f, 1.0f, 0.0f }
        };
        setCameraEye(memory, &cameraEye);

        Frustum cameraFrustum = {
            .fov    = 45.0f,
            .aspect = 1920.0f / 1080.0f,
            .near   = 1.0f,
            .far    = 100.0f
        };
        setCameraFrustum(memory, &cameraFrustum);

        updateCamera(memory);

        const char* cameraUniformNames[4] = {
            "cameraProjection",
            "cameraLocalToWorld",
            "cameraLocalRotation",
            "cameraView"
        };

        generateUniformBuffer(memory, 0, "CameraMatrices", cameraUniformNames);
        mapCameraUniforms(memory);

        generateRenderBatch(memory, 0, 0, 0, 0);
        setVertexLayout(memory, 0, 0);
    }

    void freeGraphicsResources(Allocator* allocator)
    {
        freeShaders(allocator);
        freeTextures(allocator);
        freeVertexArrays(allocator);
        freeBuffers(allocator);
    }

    void renderBatches(const Allocator* allocator)
    {
        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        ConstMemoryView memoryView(allocatorSpan);
        MemoryCursor<MEMORY_ALIGNMENT> batchCursor(getRenderBatchOffset(allocator));

        const auto batchCount = memoryView.read_object<uint64_t>(batchCursor.getOffset());
        batchCursor.step<uint64_t>();

        for (uint64_t i = 0; i < *batchCount.data(); ++i)
        {
            const auto* batch = memoryView.read_object<RenderBatch>(batchCursor.getOffset()).data();

            const auto locationsDescriptors = memoryView.read_contiguous_array<LocationsDescriptor>(getLocationsDescriptorOffset(allocator));
            const auto* locationsDescriptor = locationsDescriptors.data() + batch->descriptorIndex;
            
            renderBatch(batch);

            batchCursor.step<RenderBatch>();

            const auto renderEntities   = memoryView.read_contiguous_array<RenderEntity>(batchCursor.getOffset());
            const size_t entityCount    = renderEntities.size();

            for (size_t j = 0; j < entityCount; ++j)
            {
                renderEntity(renderEntities.data() + j, locationsDescriptor, batch->elememtCount);
            }

            batchCursor.step_array<RenderEntity>(entityCount);
        }
    }

    void render(const Allocator* allocator)
    {
        // uploadUniformBuffer(allocator);
        // renderBatches(allocator);
    }
}