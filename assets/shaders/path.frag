#version 330 core
out vec4 FragColor;

in float f_distance;
in float f_totalDistance;

uniform float u_progress = 1.0;
uniform vec4 v_color;

void main() {
	float progress = f_distance / f_totalDistance;

    if (progress > u_progress) {
        discard;
    }

    FragColor = v_color;
}
