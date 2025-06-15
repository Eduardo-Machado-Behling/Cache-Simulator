#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 m_model;
uniform mat4 m_projection;

// Pass a color-like value to the fragment shader
out vec3 vertexColor;

void main() {
  gl_Position = m_projection * m_model * vec4(aPos, 1.0);

  // Pass the raw model-space position as a color.
  // We expect a gradient from black (0,0) to yellow (1,1).
  vertexColor = gl_Position.xyz;
}