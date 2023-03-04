#version 410 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 fragColor;

in vec3 vFragPosition;
in vec3 vNormal;
in vec2 vTexCoord;

#define POINT_LIGHTS_NUM 1

uniform vec3 viewPosition;
uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[POINT_LIGHTS_NUM];
uniform SpotLight spotLight;

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDirection);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDirection);

void main() {
    vec3 normal = normalize(vNormal);
    vec3 viewDirection = normalize(viewPosition - vFragPosition);

    vec3 result = calculateDirectionalLight(directionalLight, normal, viewDirection);

    for (int i = 0; i < POINT_LIGHTS_NUM; ++i) {
        result += calculatePointLight(pointLights[i], normal, vFragPosition, viewDirection);
    }

    result += calculateSpotLight(spotLight, normal, vFragPosition, viewDirection);

    fragColor = vec4(result, 1.0);
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection) {
    vec3 lightDirection = normalize(-light.direction);

    // diffuse
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

    vec3 ambient = light.ambient * texture(material.diffuse, vTexCoord).rgb;
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, vTexCoord).rgb;
    vec3 specular = light.specular * spec * texture(material.specular, vTexCoord).rgb;
    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDirection) {
    vec3 lightDirection = normalize(light.position - fragPos);

    // diffuse
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * texture(material.diffuse, vTexCoord).rgb;
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, vTexCoord).rgb;
    vec3 specular = light.specular * spec * texture(material.specular, vTexCoord).rgb;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDirection) {
    vec3 lightDirection = normalize(light.position - fragPos);

    // diffuse
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // intensity
    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * texture(material.diffuse, vTexCoord).rgb;
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, vTexCoord).rgb;
    vec3 specular = light.specular * spec * texture(material.specular, vTexCoord).rgb;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return ambient + diffuse + specular;
}
