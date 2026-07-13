#include <glad/glad.h>

#include <memory_cursor.hpp>
#include <memory_view.hpp>

#include "opengl_allocator.h"
#include "platform.h"
#include "renderer.h"

namespace
{
    constexpr size_t MAX_SHADER_FILE_SIZE = 1 << 10;
}

namespace renderer
{
    void generateBuffers(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView bufferView(allocatorSpan);
        auto buffers = bufferView.read_contiguous_array<unsigned int>(getBufferOffset(allocator));

        OpenGLAllocator bufferAllocator(glGenBuffers, glDeleteBuffers, buffers.data(), buffers.data());
        bufferAllocator.allocate(static_cast<GLsizei>(buffers.size()));
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

    void generateVertexArrays(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView vertexArrayView(allocatorSpan);
        auto vertexArrays = vertexArrayView.read_contiguous_array<unsigned int>(getVertexArrayOffset(allocator));

        OpenGLAllocator vertexArrayAllocator(glGenVertexArrays, glDeleteVertexArrays, vertexArrays.data(), vertexArrays.data());
        vertexArrayAllocator.allocate(static_cast<GLsizei>(vertexArrays.size()));
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

    void generateTextures(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView textureView(allocatorSpan);
        auto textures = textureView.read_contiguous_array<unsigned int>(getTextureOffset(allocator));

        OpenGLAllocator textureAllocator(glGenTextures, glDeleteTextures, textures.data(), textures.data());
        textureAllocator.allocate(static_cast<GLsizei>(textures.size()));
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

    void generateMeshes(Allocator* allocator)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);
        auto buffers = memoryView.read_contiguous_array<unsigned int>(getBufferOffset(allocator));

        auto* bufferData            = buffers.data();
        const size_t bufferCount    = buffers.size();
        
        OpenGLAllocator bufferAllocator(glGenBuffers, glDeleteBuffers, bufferData, bufferData + bufferCount);

        MemoryCursor<MEMORY_ALIGNMENT> cursor(getMeshDataOffset(allocator));
        auto meshCount = memoryView.read_object<uint64_t>(cursor.getOffset());
        cursor.step<uint64_t>();

        for (size_t i = 0; i < *meshCount.data(); ++i)
        {
            auto* bufferIndices = memoryView.read_object<ConstMesh::BufferIndices>(cursor.getOffset()).data();

            bufferIndices->vertex   = bufferAllocator.get();
            bufferIndices->triangle = bufferAllocator.get();

            cursor.step<ConstMesh::BufferIndices>();

            auto* vertexCount = memoryView.read_object<uint64_t>(cursor.getOffset()).data();
            cursor.step_array<Vertex>(*vertexCount);

            auto* triangleCount = memoryView.read_object<uint64_t>(cursor.getOffset()).data();
            cursor.step_array<unsigned int>(*triangleCount);
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

    void generateShaders(Allocator* allocator, size_t count, Shader* shaders)
    {
        auto allocatorSpan = std::span<std::byte, ALLOCATOR_SIZE>(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);
        MutableMemoryView memoryView(allocatorSpan);
        auto* shaderData = memoryView.read_contiguous_array<Shader>(getShaderOffset(allocator)).data();

        for (size_t i = 0; i < count; ++i)
        {
            shaderData[i] = shaders[i];

            Shader& shader = shaderData[i];

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

            size_t fileSize = platform::getFileSize(shader.path);
            if (fileSize == 0)
                continue;

            char* fileBuffer = new char[fileSize + 1]();
            std::memset(fileBuffer, 0, fileSize + 1);
            if (!platform::readFile(shader.path, fileBuffer, fileSize))
            {
                delete[] fileBuffer;
                continue;
            }

            glShaderSource(shader.index, 1, &fileBuffer, nullptr);
            glCompileShader(shader.index);

            delete[] fileBuffer;
        }
    }

    void generateShaderPrograms(Allocator* allocator)
    {
        auto allocatorSpan = std::span<std::byte, ALLOCATOR_SIZE>(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);
        MutableMemoryView memoryView(allocatorSpan);
        MemoryCursor<16> cursor(getShaderProgramOffset(allocator));

        auto* programCount  = memoryView.read_object<uint64_t>(cursor.getOffset()).data();
        auto* programData   = memoryView.read_contiguous_array<unsigned int>(cursor.getOffset()).data();

        for (size_t i = 0; i < *programCount; ++i)
        {
            programData[i] = glCreateProgram();
        }
    }

    void compileShaderProgram(const Allocator* allocator, size_t index, size_t shaderCount, const size_t* shaderIndices)
    {
        auto allocatorSpan = std::span<const std::byte, ALLOCATOR_SIZE>(reinterpret_cast<const std::byte*>(allocator->peek()), ALLOCATOR_SIZE);
        ConstMemoryView memoryView(allocatorSpan);
        MemoryCursor<16> shaderCursor(getShaderOffset(allocator));

        const auto shaderSize = memoryView.read_object<uint64_t>(shaderCursor.getOffset());
        const auto shaderData = memoryView.read_contiguous_array<Shader>(shaderCursor.getOffset());

        if (shaderCount > *shaderSize.data())
            return;

        MemoryCursor<16> programCursor(getShaderProgramOffset(allocator));
        const auto programCount     = memoryView.read_object<uint64_t>(programCursor.getOffset());
        const auto programData      = memoryView.read_contiguous_array<unsigned int>(programCursor.getOffset());

        if (index >= *programCount.data())
            return;

        const unsigned int program = programData.data()[index];

        for (size_t i = 0; i < shaderCount; ++i)
        {
            const size_t shaderIndex = shaderIndices[i];

            if (shaderIndex >= *shaderSize.data())
                continue;

            const Shader& shader = shaderData.data()[shaderIndex];

            glAttachShader(program, shader.index);
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

        // Hot pink!
        glClearColor(1.0f, 0.4118f, 0.7059f, 1.0f);
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

    bool setShaderLocations(Allocator* allocator, size_t programIndex, size_t descriptorIndex)
    {
        std::span<std::byte, ALLOCATOR_SIZE> allocatorSpan(reinterpret_cast<std::byte*>(allocator->peek()), ALLOCATOR_SIZE);

        MutableMemoryView memoryView(allocatorSpan);

        const auto shaderPrograms = memoryView.read_contiguous_array<unsigned int>(getShaderProgramOffset(allocator));

        if (programIndex >= shaderPrograms.size())
        {
            return false;
        }

        const GLuint shaderProgram = shaderPrograms.data()[programIndex];

        auto locationsDescriptors = memoryView.read_contiguous_array<LocationsDescriptor>(getLocationsDescriptorOffset(allocator));

        if (descriptorIndex >= locationsDescriptors.size())
        {
            return false;
        }

        auto* locationsDescriptor = locationsDescriptors.data() + descriptorIndex;

        locationsDescriptor->positionLocation       = glGetAttribLocation(shaderProgram, "position");
        locationsDescriptor->colorLocation          = glGetAttribLocation(shaderProgram, "color");
        locationsDescriptor->uvLocation             = glGetAttribLocation(shaderProgram, "uv");
        locationsDescriptor->transformLocation      = glGetUniformLocation(shaderProgram, "transform");
        locationsDescriptor->materialColorLocation  = glGetUniformLocation(shaderProgram, "materialColor");

        return true;
    }
}