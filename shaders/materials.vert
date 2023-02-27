#version 410 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

out vec3 vFragPosition;
out vec3 vNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vFragPosition = vec3(model * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * vec4(vFragPosition, 1.0);
}
