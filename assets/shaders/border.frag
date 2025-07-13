#version 330 core
out vec4 FragColor;

// Interpolated UV coordinate from vertex shader
in vec2 v_uv;

// Uniforms to control the border's appearance
uniform vec4 u_borderColor = vec4(0.1, 0.1, 0.1, 1.0); // Dark grey
uniform vec4 u_fillColor = vec4(0.0, 0.5, 0.5, 1.0); // Teal

// Uniforms for absolute pixel calculations
uniform vec2 u_rectPixelSize; // The size of the rect in pixels
uniform float u_borderPixelWidth = 10.0; // The desired border width in pixels
uniform float u_progress = 1.0;

void main() {
    // Convert texture coordinates to pixel coordinates
    vec2 pixelCoord = v_uv * u_rectPixelSize;

    if (v_uv.y > u_progress) {
        discard;
    }

    // Check if we're in any border region
    bool inLeftBorder = pixelCoord.x < u_borderPixelWidth;
    bool inRightBorder = pixelCoord.x > (u_rectPixelSize.x - u_borderPixelWidth);
    bool inBottomBorder = pixelCoord.y < u_borderPixelWidth;
    bool inTopBorder = pixelCoord.y > (u_rectPixelSize.y - u_borderPixelWidth);

    bool inBorder = inLeftBorder || inRightBorder || inBottomBorder || inTopBorder;

    FragColor = inBorder ? u_borderColor : u_fillColor;
}
