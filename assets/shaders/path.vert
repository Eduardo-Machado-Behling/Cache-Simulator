#version 330 core
layout(location = 0) in vec3 aPos; // Vertex position (x, y)
layout(location = 1) in float aDistance; // Vertex position (x, y)
layout(location = 2) in float aTotalDistance; // Vertex position (x, y)

uniform mat4 m_model;
uniform mat4 m_projection;
uniform mat4 m_view;

out float f_distance;
out float f_totalDistance;

void main() {
    gl_Position = m_projection * m_view * m_model * vec4(aPos, 1.0);
    f_distance = aDistance;
    f_totalDistance = aTotalDistance;
}
