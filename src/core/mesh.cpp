#include "mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : vertices(vertices), indices(indices)
{
    
}

const std::vector<Vertex>& Mesh::getVertices() const
{
    return vertices;
}

const std::vector<uint32_t>& Mesh::getIndices() const
{
    return indices;
}