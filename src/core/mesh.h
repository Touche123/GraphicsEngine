#pragma once

#include "vertex.h"
#include <vector>
#include "../graphics/GLVertexArray.h"

struct Mesh
{
    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

    void Clear();

    auto GetTriangleCount() const noexcept { return IndexCount / 3; }

    const std::size_t IndexCount;
    GLVertexArray VAO;

    /*const std::vector<Vertex>& getVertices() const;

    const std::vector<uint32_t>& getIndices() const;*/

private:
    /*const std::vector<Vertex> vertices;
    const std::vector<uint32_t> indices;*/

    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);


};

