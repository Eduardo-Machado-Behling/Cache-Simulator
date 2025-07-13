#version 330 core
layout (location = 0) in vec3 a_pos;

uniform mat4 m_model;
uniform mat4 m_projection;

out vec2 v_uv;

void main() {
    gl_Position = m_projection * m_model * vec4(a_pos, 1.0);

    // It remaps [-1, 1] to [0, 1].
    v_uv = a_pos.xy;
}
