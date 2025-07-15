#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // Texture coordinates from vertex shader

uniform sampler2D ourTexture; // Declare a uniform for the texture sampler

void main() {
	vec4 color =texture(ourTexture, TexCoord); // Sample the texture 
	if(color.a == 0){
		discard;
	}

    FragColor = color;
}
