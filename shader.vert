#version 410 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
uniform mat4 transform;
out vec2 vTexCoord;
void main() {
    gl_Position = transform * vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
