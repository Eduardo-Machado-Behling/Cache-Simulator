#version 330 core
layout(location = 0) in vec3 aPos; // Vertex position (x, y)
layout(location = 1) in float aDistance; // Vertex position (x, y)
layout(location = 2) in float aTotalDistance; // Vertex position (x, y)

uniform mat4 m_model;
uniform mat4 m_projection;
uniform mat4 m_view;
uniform vec3 v_origin = vec3(0,0,-1);

out float f_distance;
out float f_totalDistance;

void main() {
	vec4 vertex = vec4(aPos, 1.0);
	if(v_origin.z != -1 && aDistance == 0){
		vertex = vec4(v_origin, 1.0);
	}

    gl_Position = m_projection * m_view * m_model * vertex;
    f_distance = aDistance;
    f_totalDistance = aTotalDistance;
}
