#pragma once

#include "Mesh.h"
#include <string>

namespace Assets
{
    Mesh loadOBJFile(const char* path, const char* basePath, bool triangulate);
    std::string loadTextFile(const std::string& path);
}

