#version 410 core

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

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

    float specularStrength = 0.75;
    vec3 view_dir = normalize(cameraPosition - vFragPosition);
    vec3 reflect_dir = reflect(-light_dir, normal);
    vec3 specular = specularStrength * pow(max(dot(view_dir, reflect_dir), 0.0), 32) * lightColor;

    fragColor = vec4(objectColor * (ambient + diffuse + specular), 1.0);
}
