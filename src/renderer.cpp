#include <glm/gtc/matrix_transform.hpp>

#include <memory_cursor.hpp>
#include <memory_view.hpp>

#include "renderer.h"

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
            cursor.step<ConstMesh::BufferIndices>();

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
        cursor.step<Camera::Eye>();
        return cursor.getOffset();
    }

    uint64_t getCameraOffset(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> cursor(getCameraFrustumOffset(allocator));
        cursor.step<Camera::Frustum>();
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

    void allocateMeshes(Allocator* allocator, size_t count, const ConstMesh* meshes)
    {
        auto* meshCount = allocator->requestMemory<uint64_t>(count);
        
        for (size_t i = 0; i < *meshCount; ++i)
        {
            const ConstMesh& mesh = meshes[i];

            auto* bufferIndices = allocator->requestMemory<ConstMesh::BufferIndices>();
            *bufferIndices      = mesh.bufferIndices;

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

    void uploadMeshes(const Allocator* allocator)
    {
        MemoryCursor<MEMORY_ALIGNMENT> meshCursor(getMeshDataOffset(allocator));

        std::span<const std::byte, ALLOCATOR_SIZE> meshSpan(
            reinterpret_cast<const std::byte*>(allocator->peek()),
            ALLOCATOR_SIZE);

        ConstMemoryView meshMemoryView(meshSpan);

        const auto* meshCount = meshMemoryView.read_object<uint64_t>(meshCursor.getOffset()).data();
        meshCursor.step<uint64_t>();

        for (uint64_t i = 0; i < *meshCount; ++i)
        {
            const auto* bufferIndices = meshMemoryView.read_object<ConstMesh::BufferIndices>(meshCursor.getOffset()).data();
            meshCursor.step<ConstMesh::BufferIndices>();

            const auto* vertexCount = meshMemoryView.read_object<uint64_t>(meshCursor.getOffset()).data();
            const auto vertexData   = meshMemoryView.read_contiguous_array<Vertex>(meshCursor.getOffset());
            meshCursor.step_array<Vertex>(*vertexCount);

            const auto* triangleCount   = meshMemoryView.read_object<uint64_t>(meshCursor.getOffset()).data();
            const auto triangleData     = meshMemoryView.read_contiguous_array<unsigned int>(meshCursor.getOffset());
            meshCursor.step_array<unsigned int>(*triangleCount);

            ConstMesh mesh{ vertexData.data(), triangleData.data(), *vertexCount, *triangleCount, *bufferIndices };
            uploadMesh(&mesh);
        }
    }

    void allocateShaders(Allocator* allocator, size_t count)
    {
        allocator->requestMemory<uint64_t>(count);
        allocator->requestMemoryArray<Shader>(count);
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
        allocator->requestMemory<Camera::Eye>();
        allocator->requestMemory<Camera::Frustum>();
        allocator->requestMemory<Camera>();
    }

    void setCameraEye(Allocator* allocator, const Camera::Eye* eye)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MemoryCursor<MEMORY_ALIGNMENT> cursor(getCameraEyeOffset(allocator));

        MutableMemoryView memoryView(allocatorSpan);
        auto* cameraEye = memoryView.read_object<Camera::Eye>(cursor.getOffset()).data();

        *cameraEye = *eye;
    }

    void setCameraFrustum(Allocator* allocator, const Camera::Frustum* frustum)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MemoryCursor<MEMORY_ALIGNMENT> cursor(getCameraFrustumOffset(allocator));

        MutableMemoryView memoryView(allocatorSpan);
        auto* cameraFrustum = memoryView.read_object<Camera::Frustum>(cursor.getOffset()).data();

        *cameraFrustum = *frustum;
    }

    void updateCamera(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MemoryCursor<MEMORY_ALIGNMENT> cameraDataCursor(getCameraEyeOffset(allocator));

        ConstMemoryView cameraDataView(allocatorSpan);

        const auto* cameraEye = cameraDataView.read_object<Camera::Eye>(cameraDataCursor.getOffset()).data();
        cameraDataCursor.step<Camera::Eye>();

        const auto* cameraFrustum = cameraDataView.read_object<Camera::Frustum>(cameraDataCursor.getOffset()).data();
        cameraDataCursor.step<Camera::Frustum>();

        MutableMemoryView cameraView(allocatorSpan);
        MemoryCursor<MEMORY_ALIGNMENT> cameraCursor(getCameraOffset(allocator));
        auto* camera = cameraView.read_object<Camera>(cameraCursor.getOffset()).data();

        camera->projection  = glm::perspective(glm::radians(cameraFrustum->fov), cameraFrustum->aspect, cameraFrustum->near, cameraFrustum->far);
        camera->view        = glm::lookAtRH(cameraEye->position, cameraEye->target, cameraEye->up);
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
        ConstMesh meshList[1]           = { { quadVertices, quadTriangles, 4, 6 } };
        allocateMeshes(allocator, 1, meshList);
        allocateLocationsDescriptors(allocator, 1);
        allocateCamera(allocator);
    }

    void initializeGraphicsResources(Allocator* allocator)
    {
        generateBuffers(allocator);
        generateVertexArrays(allocator);
        generateTextures(allocator);

        Shader shaderList[2] = {
            { "./shaders/3d_transform_vertex.slh", Shader::Type::VERTEX },
            { "./shaders/3d_transform_fragment.slh", Shader::Type::FRAGMENT },
        };
        generateShaders(allocator, 2, shaderList);
        
        size_t shaderIndices[2] = { 0, 1 };
        generateShaderPrograms(allocator);
        compileShaderProgram(allocator, 0, 2, shaderIndices);

        generateMeshes(allocator);
        uploadMeshes(allocator);

        setShaderLocations(allocator, 0, 0);

        Camera::Eye cameraEye = {
            .position   = glm::vec3{ 0.0f, 0.0f, 10.0f },
            .target	    = glm::vec3{ 0.0f, 0.0f, 0.0f },
            .up		    = glm::vec3{ 0.0f, 1.0f, 0.0f }
        };
        setCameraEye(allocator, &cameraEye);

        Camera::Frustum cameraFrustum = {
            .fov    = 45.0f,
            .aspect = 1920.0f / 1080.0f,
            .near   = 1.0f,
            .far    = 100.0f
        };
        setCameraFrustum(allocator, &cameraFrustum);

        updateCamera(allocator);
    }

    void freeGraphicsResources(Allocator* allocator)
    {
        freeShaders(allocator);
        freeTextures(allocator);
        freeVertexArrays(allocator);
        freeBuffers(allocator);
    }

    void render(const Allocator* allocator)
    {
        // const auto* allocatorStart = allocator->peek();
        // std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocatorStart), ALLOCATOR_SIZE);

        // auto memoryCursor = MemoryCursor<MEMORY_ALIGNMENT>();

        // ConstMemoryView allocatorMemoryView(allocatorSpan);
        
        // const auto locationsDescriptor =
        //     allocatorMemoryView.read_object<LocationsDescriptor>(memoryCursor.getOffset());
        // memoryCursor.step<LocationsDescriptor>();
    }
}