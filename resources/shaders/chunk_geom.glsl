
/**
 * Geometry shader for chunk rendering. Accepts a point primitive for each chunk
 * and expands it into a square, the first point represents the top left
 * coordinate. Each of the new vertices in the square is given a relative
 * position so that the fragment shader can determine which side of the square
 * a fragment is on.
 */

#version 150

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform vec2 positionOffset;
uniform vec2 texCoordsOffset;

in vec2 gTexCoords[];
in vec4 gColor[];

out vec2 fTexCoords;
out vec4 fColor;
out vec2 fRelativePos;

void main() {
    fColor = gColor[0];

    gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.0, 0.0, 0.0);
    fTexCoords = gTexCoords[0] + vec2( 0.0,  0.0);
    fRelativePos = vec2(0.0, 0.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(positionOffset.x, 0.0, 0.0, 0.0);
    fTexCoords = gTexCoords[0] + vec2(texCoordsOffset.x, 0.0);
    fRelativePos = vec2(1.0, 0.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(0.0, positionOffset.y, 0.0, 0.0);
    fTexCoords = gTexCoords[0] + vec2(0.0, -texCoordsOffset.y);
    fRelativePos = vec2(0.0, 1.0);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(positionOffset.x, positionOffset.y, 0.0, 0.0);
    fTexCoords = gTexCoords[0] + vec2(texCoordsOffset.x, -texCoordsOffset.y);
    fRelativePos = vec2(1.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
