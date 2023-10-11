#include "Assets.h"

#include "vertex.h"

#include "../../tiny_obj_loader.h"

#include <stdexcept>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <unordered_map>

Mesh Assets::loadOBJFile(const char* path, const char* basePath, bool triangulate)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, path,
        basePath, triangulate))
    {
        throw std::runtime_error("ast::assets::loadOBJFile: Error: " + warning + error);
    }
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<glm::vec3, uint32_t> uniqueVertices;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            glm::vec3 position{
                attributes.vertices[3 * index.vertex_index + 0],
                attributes.vertices[3 * index.vertex_index + 1],
                attributes.vertices[3 * index.vertex_index + 2] };

            if (uniqueVertices.count(position) == 0)
            {
                uniqueVertices[position] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(Vertex{ position });
            }

            indices.push_back(uniqueVertices[position]);
        }
    }

    return Mesh{ vertices, indices };
}

std::string Assets::loadTextFile(const std::string& path)
{
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();

    return buffer.str();
}