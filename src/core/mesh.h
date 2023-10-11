#pragma once

#include "vertex.h"
#include <vector>

struct Mesh
{
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    const std::vector<Vertex>& getVertices() const;

    const std::vector<uint32_t>& getIndices() const;

private:
    const std::vector<Vertex> vertices;
    const std::vector<uint32_t> indices;
};

