#pragma once

#include "glm-wrapper.h"

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    bool operator==(const Vertex& other) const;
};

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(const Vertex& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec2>()(vertex.TexCoords) << 1)) >> 1);
        }
    };
}