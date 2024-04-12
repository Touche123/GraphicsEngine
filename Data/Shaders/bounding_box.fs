#version 330 core

out vec4 FragColor;

uniform vec3 minExtents;
uniform vec3 maxExtents;

void main() {

    // Example: Check if the fragment lies within the bounding box
    if (gl_FragCoord.x >= minExtents.x && gl_FragCoord.x <= maxExtents.x &&
        gl_FragCoord.y >= minExtents.y && gl_FragCoord.y <= maxExtents.y &&
        gl_FragCoord.z >= minExtents.z && gl_FragCoord.z <= maxExtents.z) {
        // Inside bounding box
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color
    } else {
        // Outside bounding box
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color
        //discard; // Discard the fragment
    }

    //FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color

    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}