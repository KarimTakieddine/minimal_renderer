#include <span>

#include <memory_cursor.hpp>
#include <memory_view.hpp>

#include "locations_descriptor.h"
#include "mesh.h"
#include "renderer.h"

namespace
{
    constexpr size_t ALLOCATOR_SIZE         = 1 << 13;
    constexpr uint64_t MEMORY_ALIGNMENT     = 16;
    constexpr uint64_t MESH_SECTION_SIZE    = 1 << 10;
}

namespace renderer
{
    void allocateMeshes(Allocator* allocator, size_t count, const Mesh* meshes)
    {
        auto* meshCount = allocator->requestMemory<uint64_t>(count);
        
        for (size_t i = 0; i < *meshCount; ++i)
        {
            const Mesh& mesh = meshes[i];

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
        MemoryCursor<MEMORY_ALIGNMENT> meshCursor;
        
        std::span<const std::byte, MESH_SECTION_SIZE> meshSpan(
            static_cast<const std::byte*>(allocator->peek()),
            MESH_SECTION_SIZE);

        ConstMemoryView meshMemoryView(meshSpan);

        const auto* meshCount = meshMemoryView.read_object<uint64_t>(meshCursor.getOffset()).data();
        meshCursor.step<uint64_t>();

        for (uint64_t i = 0; i < *meshCount; ++i)
        {
            const auto* vertexCount = meshMemoryView.read_object<uint64_t>(meshCursor.getOffset()).data();
            const auto vertexData   = meshMemoryView.read_contiguous_array<Vertex>(meshCursor.getOffset());
            meshCursor.step_array<Vertex>(*vertexCount);

            const auto* triangleCount   = meshMemoryView.read_object<uint64_t>(meshCursor.getOffset()).data();
            const auto triangleData     = meshMemoryView.read_contiguous_array<unsigned int>(meshCursor.getOffset());
            meshCursor.step_array<unsigned int>(*triangleCount);
        }
    }

    void allocate(Allocator* allocator)
    {
        allocator->allocate(ALLOCATOR_SIZE);

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
        Mesh quadMesh[2] = { { quadVertices, quadTriangles, 4, 6 }, { quadVertices, quadTriangles, 4, 6 } };
        allocateMeshes(allocator, 2, quadMesh);
    }

    void render(const Allocator* allocator)
    {
        uploadMeshes(allocator);

        // const auto* allocatorStart = allocator->peek();
        // std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocatorStart), ALLOCATOR_SIZE);

        // auto memoryCursor = MemoryCursor<MEMORY_ALIGNMENT>();

        // ConstMemoryView allocatorMemoryView(allocatorSpan);
        
        // const auto locationsDescriptor =
        //     allocatorMemoryView.read_object<LocationsDescriptor>(memoryCursor.getOffset());
        // memoryCursor.step<LocationsDescriptor>();
    }
}