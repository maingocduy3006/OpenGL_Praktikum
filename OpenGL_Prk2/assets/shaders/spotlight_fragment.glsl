#version 330 core

in vec3 ColorVS;
in vec3 PositionVS;
in vec3 NormalVS;

uniform vec3 viewPosition;
uniform vec3 colorAdjustment;

out vec4 color;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

struct Material {
    vec3 specular;
    float shininess;
};

struct Spotlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float phi;   // Inner cone angle
    float gamma; // Outer cone angle
    float kc;    // Constant attenuation
    float kl;    // Linear attenuation
    float kq;    // Quadratic attenuation
};

uniform Light light;
uniform Spotlight spotlight;
uniform Material material;
void main() {
    // Adjust color and clamp
    vec3 adjustedColor = clamp(ColorVS + colorAdjustment, 0.0, 1.0);

    // Lighting calculations
    vec3 norm = normalize(NormalVS);
    vec3 lightDir = normalize(light.position - PositionVS);
    vec3 viewDir = normalize(viewPosition - PositionVS);

    float cosTheta = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.intensity * cosTheta * light.color * adjustedColor;

    vec3 reflectDir = reflect(-lightDir, norm);
    float cosAlpha = max(dot(viewDir, reflectDir), 0.0);
    float spec = pow(cosAlpha, material.shininess);
    vec3 specular = material.specular * spec * light.color * light.intensity;

    vec3 ambient = globalAmbient * adjustedColor;

    // Apply a different shading condition for the floor
    if (PositionVS.y < -0.4) {
        vec3 floorColor = vec3(0.2, 0.2, 0.2); // Black color for the floor

        // Lighting calculations for the floor
        vec3 norm = normalize(NormalVS);
        vec3 lightDir = normalize(light.position - PositionVS);
        vec3 viewDir = normalize(viewPosition - PositionVS);

        float cosTheta = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.intensity * cosTheta * light.color * floorColor;

        vec3 reflectDir = reflect(-lightDir, norm);
        float cosAlpha = max(dot(viewDir, reflectDir), 0.0);
        float spec = pow(cosAlpha, material.shininess);
        vec3 specular = material.specular * spec * light.color * light.intensity;

        vec3 ambient = globalAmbient * floorColor;

        vec3 result = ambient + diffuse + specular;
        color = vec4(result, 1.0);
    } else {
        // Lighting calculations for other objects
        vec3 result = ambient + diffuse + specular;
        color = vec4(result, 1.0);
    }

}
