#include "mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : IndexCount(indices.size())
{
    setupMesh(vertices, indices);
}

void Mesh::setupMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
{
    VAO.Init();
    VAO.Bind();

    VAO.AttachBuffer(GLVertexArray::BufferType::ARRAY, vertices.size() * sizeof(Vertex), GLVertexArray::DrawMode::STATIC, &vertices[0]);

    VAO.AttachBuffer(GLVertexArray::BufferType::ELEMENT, indices.size() * sizeof(GLuint), GLVertexArray::DrawMode::STATIC, &indices[0]);

    const static auto vertexSize = sizeof(Vertex);

    VAO.EnableAttribute(0, 3, vertexSize, nullptr);
    VAO.EnableAttribute(1, 3, vertexSize, reinterpret_cast<void*>(offsetof(Vertex, Normal)));
    VAO.EnableAttribute(2, 2, vertexSize, reinterpret_cast<void*>(offsetof(Vertex, TexCoords)));
}

//// vertex positions
//glEnableVertexAttribArray(0);
//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
//// vertex normals
//glEnableVertexAttribArray(1);
//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
//// vertex texture coords
//glEnableVertexAttribArray(2);
//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));