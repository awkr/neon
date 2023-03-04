#version 410 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // attenuation
    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

out vec4 fragColor;

in vec3 vFragPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 viewPosition;
uniform Material material;
uniform Light light;

void main() {
    // ambient
    vec3 ambient = texture(material.diffuse, vTexCoord).rgb * light.ambient;

    // diffuse
    vec3 normal = normalize(vNormal);
    vec3 light_dir = normalize(light.position - vFragPosition);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = (texture(material.diffuse, vTexCoord).rgb * diff) * light.diffuse;

    // specular
    vec3 view_dir = normalize(viewPosition - vFragPosition);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (texture(material.specular, vTexCoord).rgb * spec) * light.specular;

    // spotlight (soft edge)
    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    // attenuation
    float distance = length(light.position - vFragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
