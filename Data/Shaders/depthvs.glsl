#version 440 core

layout (location = 0) in vec3 position;

layout (std140, binding = 0) uniform Matrices {
  mat4 projection;
  mat4 view;
};

out vec3 FragPos;

uniform mat4 modelMatrix;

void main() {
    vec4 viewPos = view * modelMatrix * vec4(position, 1.0);
    FragPos = viewPos.xyz;
    
    gl_Position = projection * view * modelMatrix * vec4(position, 1.0);
}  