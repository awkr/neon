#version 410 core

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;

out vec4 fragColor;

in vec3 vNormal;
in vec3 vFragPosition;

void main() {
    float ambientStrength = 0.075;
    vec3 ambient = ambientStrength * lightColor;

    vec3 normal = normalize(vNormal);
    vec3 light_dir = normalize(lightPosition - vFragPosition);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * lightColor;

    fragColor = vec4(objectColor * (ambient + diffuse), 1.0);
}
