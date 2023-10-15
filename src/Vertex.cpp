#include "Vertex.h"

bool Vertex::operator==(const Vertex& other) const
{
    return Position == other.Position && TexCoords == other.TexCoords;
}