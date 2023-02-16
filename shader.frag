#version 410 core
uniform sampler2D tex;
in vec2 vTexCoord;
out vec4 fColor;
void main() {
    fColor = texture(tex, vTexCoord);
}
