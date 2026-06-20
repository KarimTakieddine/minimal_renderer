#include <glad/glad.h>

#include <memory_view.hpp>

#include "opengl_allocator.h"
#include "renderer.h"

namespace renderer
{
    void allocateBuffers(Allocator* allocator, size_t count)
    {
        const auto* bufferCount = allocator->requestMemory<uint64_t>(count);
        auto* bufferData        = allocator->requestMemoryArray<unsigned int>(count);

        OpenGLAllocator bufferAllocator(glGenBuffers, glDeleteBuffers, bufferData);
        bufferAllocator.allocate(static_cast<GLsizei>(count));
    }

    void freeBuffers(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto buffers = memoryView.read_contiguous_array<unsigned int>(getBufferOffset(allocator));
        
        OpenGLAllocator bufferAllocator(glGenBuffers, glDeleteBuffers, buffers.data());
        bufferAllocator.free(static_cast<GLsizei>(buffers.size()));
    }

    void allocateVertexArrays(Allocator* allocator, size_t count)
    {
        const auto* vertexArrayCount    = allocator->requestMemory<uint64_t>(count);
        auto* vertexArrayData           = allocator->requestMemoryArray<unsigned int>(count);

        OpenGLAllocator vertexArrayAllocator(glGenVertexArrays, glDeleteVertexArrays, vertexArrayData);
        vertexArrayAllocator.allocate(static_cast<GLsizei>(count));
    }

    void freeVertexArrays(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto vertexArrays = memoryView.read_contiguous_array<unsigned int>(getVertexArrayOffset(allocator));
        
        OpenGLAllocator vertexArrayAllocator(glGenVertexArrays, glDeleteVertexArrays, vertexArrays.data());
        vertexArrayAllocator.free(static_cast<GLsizei>(vertexArrays.size()));
    }

    void allocateTextures(Allocator* allocator, size_t count)
    {
        const auto* textureCount    = allocator->requestMemory<uint64_t>(count);
        auto* textureData           = allocator->requestMemoryArray<unsigned int>(count);

        OpenGLAllocator textureAllocator(glGenTextures, glDeleteTextures, textureData);
        textureAllocator.allocate(static_cast<GLsizei>(count));
    }

    void freeTextures(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto textures = memoryView.read_contiguous_array<unsigned int>(getTextureOffset(allocator));
        
        OpenGLAllocator textureAllocator(glGenTextures, glDeleteTextures, textures.data());
        textureAllocator.free(static_cast<GLsizei>(textures.size()));
    }

    void uploadMesh(const ConstMesh* mesh)
    {
        const ConstMesh::BufferIndices& bufferIndices = mesh->bufferIndices;

        glBindBuffer(GL_ARRAY_BUFFER, bufferIndices.vertex);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIndices.triangle);

        glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->triangleCount * sizeof(GLuint), mesh->triangles, GL_STATIC_DRAW);
    }
}