#include "model.h"
#include "../../tiny_obj_loader.h"
#include <stdexcept>
#include <iostream>

Model::Model(const std::string_view Path, const std::string_view Name, const bool flipWindingOrder, const bool loadMaterial) : m_name(Name), m_path(Path)
{
    if (!loadModel(Path))
    {
        std::cerr << "Failed to load: " << Name << '\n';
    }
}

Model::Model(const std::string_view Name, const Mesh& mesh) noexcept :m_name(Name)
{

}

bool Model::loadModel(const std::string_view Path)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, Path.data(),
        "assets/objects", true))
    {
        throw std::runtime_error("ast::assets::loadOBJFile: Error: " + warning + error);
    }
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            glm::vec3 position{
                attributes.vertices[3 * index.vertex_index + 0],
                    attributes.vertices[3 * index.vertex_index + 1],
                    attributes.vertices[3 * index.vertex_index + 2] };

            glm::vec3 normal{
                attributes.normals[3 * index.normal_index + 0],
                    attributes.normals[3 * index.normal_index + 1],
                    attributes.normals[3 * index.normal_index + 2] };

            glm::vec2 uv{
                attributes.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attributes.texcoords[2 * index.texcoord_index + 1] };


            Vertex vertex{ position, normal, uv };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }

        m_meshes.push_back(Mesh(vertices, indices));
    }
}