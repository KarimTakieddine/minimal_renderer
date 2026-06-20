#pragma once

#include "vertex.h"

namespace renderer
{
    template<typename VertexType = Vertex, typename IndexType = unsigned int>
    struct Mesh
    {
        struct BufferIndices
        {
            unsigned int vertex     { 0 };
            unsigned int triangle   { 0 };
        };

        VertexType* vertices        { nullptr };
        IndexType* triangles        { nullptr };
        size_t vertexCount          { 0 };
        size_t triangleCount        { 0 };
        BufferIndices bufferIndices;
    };

    using MutableMesh   = Mesh<Vertex, unsigned int>;
    using ConstMesh     = Mesh<const Vertex, const unsigned int>;
}
