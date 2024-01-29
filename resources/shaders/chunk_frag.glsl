
/**
 * Fragment shader for chunk rendering. The final fragment color usually just
 * ends up the same as the sampled texel at the texture coordinates. At the
 * borders between chunks, the texture coordinates are clamped to a position
 * just before the edge of the chunk, preventing linear texture filtering from
 * sampling texels in the neighbor chunk.
 * 
 * Without this fix, texture bleeding is very apparent when the view is zoomed
 * out. I originally tried other methods to fix this like adding padding around
 * the chunk textures and using a half-pixel offset, but it wasn't the best.
 * 
 * A debug flag can be set to visualize the clamping area on one side of the
 * screen, and the bleeding problem on the other side.
 */

#version 150

const bool DEBUG = false;
const float WIDTH_BIAS = 1.0 / 512.0;

uniform sampler2D texture;
uniform vec2 bufferSize;
uniform float zoomPow2;

in vec2 fTexCoords;
in vec4 fColor;
in vec2 fRelativePos;

out vec4 fragColor;

float clampNearEdge(float coord, float edge, float relativePos) {
    float width = edge * WIDTH_BIAS * zoomPow2;
    float m = mod((width * 0.5) + coord, edge);
    if (DEBUG) {
        return m > width ? coord : (relativePos > 0.5 ? -1.0 : -2.0);
    } else {
        return m > width ? coord : (relativePos > 0.5 ? coord - m : coord - m + width);
    }
}

void main() {
    float x = clampNearEdge(fTexCoords.x, 1.0 / bufferSize.x, fRelativePos.x);
    float y = clampNearEdge(fTexCoords.y, 1.0 / bufferSize.y, 1.0 - fRelativePos.y);
    vec4 pixel = texture2D(texture, vec2(x, y));

    if (DEBUG) {
        if (x == -1.0) {
            pixel = vec4(1.0, 0.0, 0.0, 1.0);    // Red.
        } else if (x == -2.0) {
            pixel = vec4(1.0, 1.0, 0.0, 1.0);    // Yellow.
        } else if (y == -1.0) {
            pixel = vec4(0.0, 1.0, 1.0, 1.0);    // Cyan.
        } else if (y == -2.0) {
            pixel = vec4(1.0, 0.0, 1.0, 1.0);    // Magenta.
        }
        if (gl_FragCoord.x > 400.0) {
            pixel = texture2D(texture, fTexCoords);
        }
    }

    fragColor = fColor * pixel;
}
