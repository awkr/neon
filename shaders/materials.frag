#version 410 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
// vec3 position;
    vec3 direction;// directional light

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 fragColor;

in vec3 vFragPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 viewPosition;
uniform Material material;
uniform Light light;

void main() {
    // Ambient
    vec3 ambient = vec3(texture(material.diffuse, vTexCoord)) * light.ambient;

    // Diffuse
    vec3 normal = normalize(vNormal);
    // vec3 light_dir = normalize(light.position - vFragPosition);
    vec3 light_dir = normalize(-light.direction);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = (vec3(texture(material.diffuse, vTexCoord)) * diff) * light.diffuse;

    // Specular
    vec3 view_dir = normalize(viewPosition - vFragPosition);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (vec3(texture(material.specular, vTexCoord)) * spec) * light.specular;

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
