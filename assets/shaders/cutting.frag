#version 330 core
out vec4 FragColor;

uniform vec4 v_color;
uniform vec2 v_border;
uniform bool orient;

void main() { 

	if(orient){
		if(gl_FragCoord.x > v_border.x && gl_FragCoord.y > v_border.y){
			discard;
		}
	} else {
		if(gl_FragCoord.x < v_border.x && gl_FragCoord.y < v_border.y){
			discard;
		}
	}

	FragColor = v_color; 
}
