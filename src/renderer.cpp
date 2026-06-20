#include <span>

#include <memory_cursor.hpp>
#include <memory_view.hpp>

#include "locations_descriptor.h"
#include "mesh.h"
#include "renderer.h"

namespace
{
    constexpr size_t ALLOCATOR_SIZE     = 1 << 13;
    constexpr size_t MESH_SECTION_SIZE  = 1 << 11;
}

namespace renderer
{
    void allocateMeshes(Allocator* allocator, size_t count, const Mesh* meshes)
    {
        auto* meshCount = allocator->requestMemory<size_t>(count);
        
        for (size_t i = 0; i < *meshCount; ++i)
        {
            const Mesh& mesh = meshes[i];

            auto* vertexCount = allocator->requestMemory<size_t>(mesh.vertexCount);
            for (size_t j = 0; j < *vertexCount; ++j)
            {
                auto* vertex    = allocator->requestMemory<Vertex>();
                *vertex         = mesh.vertices[j];
            }

            auto* triangleCount = allocator->requestMemory<size_t>(mesh.triangleCount);
            for (size_t j = 0; j < *triangleCount; ++j)
            {
                auto* triangle  = allocator->requestMemory<unsigned int>();
                *triangle       = mesh.triangles[j]; 
            }
        }
    }

    void allocate(Allocator* allocator)
    {
        allocator->allocate(ALLOCATOR_SIZE);

        auto* locationsDescriptor = allocator->requestMemory<LocationsDescriptor>();

        locationsDescriptor->materialColorLocation = 1;
        locationsDescriptor->colorLocation = 2;
        locationsDescriptor->transformLocation = 3;
        locationsDescriptor->uvLocation = 4;
        locationsDescriptor->positionLocation = 9;

        Vertex quadVertices[4] = {
            { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
            { { -0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
            { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        };
        unsigned int quadTriangles[6] = { 0, 2, 1, 0, 3, 2 };
        Mesh quadMesh{ quadVertices, quadTriangles, 4, 6 };
        allocateMeshes(allocator, 1, &quadMesh);
    }

    void render(const Allocator* allocator)
    {
        const auto* allocatorStart = allocator->peek();
        std::span<const std::byte, ALLOCATOR_SIZE> allocatorSpan(static_cast<const std::byte*>(allocatorStart), ALLOCATOR_SIZE);

        auto memoryCursor = MemoryCursor<16>();

        ConstMemoryView allocatorMemoryView(allocatorSpan);
        
        const auto locationsDescriptor =
            allocatorMemoryView.read_object<LocationsDescriptor>(memoryCursor.getOffset());
        memoryCursor.step<LocationsDescriptor>();
    }
}