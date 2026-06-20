#include <glad/glad.h>

#include <memory_view.hpp>

#include "opengl_allocator.h"
#include "platform.h"
#include "renderer.h"

#include <iostream>

namespace
{
    constexpr size_t MAX_SHADER_FILE_SIZE = 1 << 10;
}

namespace renderer
{
    void allocateBuffers(Allocator* allocator, size_t count)
    {
        const auto* bufferCount = allocator->requestMemory<uint64_t>(count);
        auto* bufferData        = allocator->requestMemoryArray<unsigned int>(count);

        OpenGLAllocator bufferAllocator(glGenBuffers, glDeleteBuffers, bufferData, bufferData);
        bufferAllocator.allocate(static_cast<GLsizei>(count));
    }

    void freeBuffers(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto buffers = memoryView.read_contiguous_array<unsigned int>(getBufferOffset(allocator));

        auto* bufferData            = buffers.data();
        const size_t bufferCount    = buffers.size();
        
        OpenGLAllocator bufferAllocator(glGenBuffers, glDeleteBuffers, bufferData, bufferData + bufferCount);
        bufferAllocator.free(static_cast<GLsizei>(bufferCount));
    }

    void allocateVertexArrays(Allocator* allocator, size_t count)
    {
        const auto* vertexArrayCount    = allocator->requestMemory<uint64_t>(count);
        auto* vertexArrayData           = allocator->requestMemoryArray<unsigned int>(count);

        OpenGLAllocator vertexArrayAllocator(glGenVertexArrays, glDeleteVertexArrays, vertexArrayData, vertexArrayData);
        vertexArrayAllocator.allocate(static_cast<GLsizei>(count));
    }

    void freeVertexArrays(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto vertexArrays = memoryView.read_contiguous_array<unsigned int>(getVertexArrayOffset(allocator));

        auto* vertexArrayData           = vertexArrays.data();
        const size_t vertexArrayCount   = vertexArrays.size();
        
        OpenGLAllocator vertexArrayAllocator(glGenVertexArrays, glDeleteVertexArrays, vertexArrayData, vertexArrayData + vertexArrayCount);
        vertexArrayAllocator.free(static_cast<GLsizei>(vertexArrayCount));
    }

    void allocateTextures(Allocator* allocator, size_t count)
    {
        const auto* textureCount    = allocator->requestMemory<uint64_t>(count);
        auto* textureData           = allocator->requestMemoryArray<unsigned int>(count);

        OpenGLAllocator textureAllocator(glGenTextures, glDeleteTextures, textureData, textureData);
        textureAllocator.allocate(static_cast<GLsizei>(count));
    }

    void freeTextures(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto textures = memoryView.read_contiguous_array<unsigned int>(getTextureOffset(allocator));

        auto* textureData           = textures.data();
        const size_t textureCount   = textures.size();
        
        OpenGLAllocator textureAllocator(glGenTextures, glDeleteTextures, textureData, textureData + textureCount);
        textureAllocator.free(static_cast<GLsizei>(textureCount));
    }

    void generateMeshes(Allocator* allocator, size_t count, ConstMesh* meshes)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto buffers = memoryView.read_contiguous_array<unsigned int>(getBufferOffset(allocator));

        auto* bufferData            = buffers.data();
        const size_t bufferCount    = buffers.size();
        
        OpenGLAllocator bufferAllocator(glGenBuffers, glDeleteBuffers, bufferData, bufferData + bufferCount);

        for (size_t i = 0; i < count; ++i)
        {
            ConstMesh::BufferIndices& bufferIndices = meshes[i].bufferIndices;

            bufferIndices.vertex    = bufferAllocator.get();
            bufferIndices.triangle  = bufferAllocator.get();
        }
    }

    void uploadMesh(const ConstMesh* mesh)
    {
        const ConstMesh::BufferIndices& bufferIndices = mesh->bufferIndices;

        glBindBuffer(GL_ARRAY_BUFFER, bufferIndices.vertex);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIndices.triangle);

        glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->triangleCount * sizeof(GLuint), mesh->triangles, GL_STATIC_DRAW);
    }

    void generateShaders(size_t count, Shader* shaders)
    {
        for (size_t i = 0; i < count; ++i)
        {
            Shader& shader = shaders[i];

            GLenum shaderType = GL_NONE;

            switch (shader.type)
            {
            case Shader::Type::VERTEX:
                shaderType = GL_VERTEX_SHADER;
                break;
            case Shader::Type::FRAGMENT:
                shaderType = GL_FRAGMENT_SHADER;
                break;
            default:
                break;
            }

            if (shaderType == GL_NONE)
            {
                continue;
            }

            shader.index = glCreateShader(shaderType);
        }
    }

    void compileShaders(size_t count, Shader* shaders, const char* const* paths)
    {
        for (size_t i = 0; i < count; ++i)
        {
            Shader& shader      = shaders[i];
            const char* path    = paths[i];

            size_t fileSize = platform::getFileSize(path);
            if (fileSize == 0)
                continue;

            char* fileBuffer = new char[fileSize + 1]();
            std::memset(fileBuffer, 0, fileSize + 1);
            if (!platform::readFile(path, fileBuffer, fileSize))
            {
                delete[] fileBuffer;
                continue;
            }

            glShaderSource(shader.index, 1, &fileBuffer, nullptr);
            glCompileShader(shader.index);

            delete[] fileBuffer;
        }
    }

    void generateShaderPrograms(size_t count, unsigned int* programs)
    {
        for (size_t i = 0; i < count; ++i)
        {
            programs[i] = glCreateProgram();
        }
    }

    void compileShaderProgram(unsigned int program, size_t shaderCount, const Shader* shaders)
    {
        for (size_t i = 0; i < shaderCount; ++i)
        {
            glAttachShader(program, shaders[i].index);
        }

        glLinkProgram(program);

        GLint linkStatus{ GL_FALSE };
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    }

    void initializeGraphicsState()
    {
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
    }

    void clearFrameBuffer()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void freeShaders(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto shaderPrograms = memoryView.read_contiguous_array<unsigned int>(getShaderProgramOffset(allocator));

        auto* shaderProgramData         = shaderPrograms.data();
        const size_t shaderProgramCount = shaderPrograms.size();

        for (size_t i = 0; i < shaderProgramCount; ++i)
        {
            const GLuint program = shaderProgramData[i];

            GLint attachedShaderCount{ 0 };
            glGetProgramiv(program, GL_ATTACHED_SHADERS, &attachedShaderCount);

            if (attachedShaderCount > 0)
            {
                GLuint* attachedShaders = new GLuint[attachedShaderCount];
                glGetAttachedShaders(program, attachedShaderCount, &attachedShaderCount, attachedShaders);
                
                for (GLint j = 0; j < attachedShaderCount; ++j)
                {
                    const GLuint shader = attachedShaders[j];

                    glDetachShader(program, shader);
                    glDeleteShader(shader);
                }

                delete[] attachedShaders;
            }

            glDeleteProgram(program);
        }
    }
}