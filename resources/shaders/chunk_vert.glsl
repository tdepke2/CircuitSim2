
/**
 * Vertex shader for chunk rendering. Simply passes position, texture
 * coordinates, and color to the geometry shader.
 * 
 * According to the GLSL spec the built-in vertex shader inputs have been
 * removed in 1.4, so stick with 1.3 here as SFML uses them.
 */

#version 130

out vec2 gTexCoords;
out vec4 gColor;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gTexCoords = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
    gColor = gl_Color;
}
