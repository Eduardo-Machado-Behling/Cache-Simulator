#version 330 core
layout(location = 0) in vec3 aPos; // Vertex position (x, y)

uniform mat4 m_model;
uniform mat4 m_projection;
uniform mat4 m_view;

out vec2 TexCoord;

void main() {
    gl_Position = m_projection * m_view * m_model * vec4(aPos, 1.0);
    TexCoord = aPos.xy;
}
