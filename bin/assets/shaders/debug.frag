#version 330 core
out vec4 FragColor;

// Receive the color-like value from the vertex shader
in vec3 vertexColor;

void main() {
  // Directly output the value from the vertex shader as the color.
  FragColor = vec4(vertexColor.z, 0.0, 0.0, 1.0);
}