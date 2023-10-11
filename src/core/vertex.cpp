#include "vertex.h"


bool Vertex::operator==(const Vertex& other) const
{
    return position == other.position && texCoord == other.texCoord;
}