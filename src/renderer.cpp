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

    void allocateMeshes(Allocator* allocator, size_t count, const ConstMesh* meshes)
    {
        auto* meshCount = allocator->requestMemory<uint64_t>(count);
        
        for (size_t i = 0; i < *meshCount; ++i)
        {
            const ConstMesh& mesh = meshes[i];

            auto* bufferIndices = allocator->requestMemory<ConstMesh::BufferIndices>();
            *bufferIndices      = mesh.bufferIndices;

            auto* vertexCount = allocator->requestMemory<uint64_t>(mesh.vertexCount);
            for (uint64_t j = 0; j < *vertexCount; ++j)
            {
                auto* vertex    = allocator->requestMemory<Vertex>();
                *vertex         = mesh.vertices[j];
            }

            auto* triangleCount = allocator->requestMemory<uint64_t>(mesh.triangleCount);

            auto* triangles = allocator->requestMemoryArray<unsigned int>(*triangleCount);
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

    void allocateShaders(Allocator* allocator, size_t count, const Shader* shaders)
    {
        auto* shaderCount   = allocator->requestMemory<uint64_t>(count);
        auto* shaderData    = allocator->requestMemoryArray<Shader>(count);

        for (size_t i = 0; i < *shaderCount; ++i)
        {
            shaderData[i] = shaders[i];
        }
    }

    void allocateShaderPrograms(Allocator* allocator, size_t count, unsigned int* programs)
    {
        auto* programCount  = allocator->requestMemory<uint64_t>(count);
        auto* programData   = allocator->requestMemoryArray<unsigned int>(count);

        for (size_t i = 0; i < *programCount; ++i)
        {
            programData[i] = programs[i];
        }
    }

    void allocate(Allocator* allocator)
    {
        allocator->allocate(ALLOCATOR_SIZE);
        
        MemoryCursor<MEMORY_ALIGNMENT> cursor;

        allocateBuffers(allocator, 3);
        allocateVertexArrays(allocator, 1);
        allocateTextures(allocator, 1);

        Shader shaders[2] = {
            { Shader::Type::VERTEX },
            { Shader::Type::FRAGMENT }
        };

        const char* shaderFiles[2] = {
            "./shaders/3d_transform_vertex.slh",
            "./shaders/3d_transform_fragment.slh"
        };

        generateShaders(2, shaders);
        compileShaders(2, shaders, shaderFiles);
        allocateShaders(allocator, 2, shaders);

        unsigned int shaderPrograms[1] = { 0 };

        generateShaderPrograms(1, shaderPrograms);
        allocateShaderPrograms(allocator, 1, shaderPrograms);
        compileShaderProgram(shaderPrograms[0], 2, shaders);

        // auto* locationsDescriptor = allocator->requestMemory<LocationsDescriptor>();

        // locationsDescriptor->materialColorLocation = 1;
        // locationsDescriptor->colorLocation = 2;
        // locationsDescriptor->transformLocation = 3;
        // locationsDescriptor->uvLocation = 4;
        // locationsDescriptor->positionLocation = 9;

        Vertex quadVertices[4] = {
            { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
            { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        };
        unsigned int quadTriangles[6] = { 0, 2, 1, 0, 3, 2 };
        ConstMesh meshList[1] = { { quadVertices, quadTriangles, 4, 6 } };
        generateMeshes(allocator, 1, meshList);
        allocateMeshes(allocator, 1, meshList);
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