#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec3 vFragPosition;

void main() {
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    vNormal = aNormal;
    vFragPosition = vec3(model * vec4(aPosition, 1.0));
}
