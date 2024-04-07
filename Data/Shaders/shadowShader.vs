#version 330 core

// Naming scheme clarification
// mS = model space
// vS = view space
// wS = world space
// tS = tangent space

layout (location = 0) in vec3 aPos;

uniform mat4 M;

void main()
{
    gl_Position = M * vec4(aPos, 1.0);
}