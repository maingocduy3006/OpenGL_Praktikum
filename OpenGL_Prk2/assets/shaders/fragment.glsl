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
    float phi; // innerer Kegelwinkel
    float gamma; // äußerer Kegelwinkel
    float constant;
    float linear;
    float quadratic;
};

uniform Light light;
uniform Spotlight spotlight;
uniform Material material;
uniform vec3 globalAmbient;

void main() {
    // Adjust color and clamp
    vec3 adjustedColor = clamp(ColorVS + colorAdjustment, 0.0, 1.0);

    // Lighting calculations
    vec3 norm = normalize(NormalVS);
    vec3 lightDir = normalize(spotlight.position - PositionVS);
    vec3 viewDir = normalize(viewPosition - PositionVS);

    float cosTheta = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = spotlight.intensity * cosTheta * spotlight.color * adjustedColor;

    vec3 reflectDir = reflect(-lightDir, norm);
    float cosAlpha = max(dot(viewDir, reflectDir), 0.0);
    float spec = pow(cosAlpha, material.shininess);
    vec3 specular = material.specular * spec * spotlight.color * spotlight.intensity;

    vec3 ambient = globalAmbient * adjustedColor;

    // Spotlight intensity calculation
    float theta = acos(dot(-spotlight.direction, lightDir));
    float phi = spotlight.phi;
    float gamma = spotlight.gamma;
    float intensityFactor = (cos(theta) - cos(gamma)) / (cos(phi) - cos(gamma));

    // Apply the spotlight intensity modulation
    vec3 spotlightColor = spotlight.color * spotlight.intensity * intensityFactor;

    // Calculate light attenuation
    float distance = length(spotlight.position - PositionVS);
    float attenuation = 1.0 / (spotlight.constant + spotlight.linear * distance + spotlight.quadratic * (distance * distance));

    // Apply light attenuation to diffuse and specular components
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *=attenuation;

    // Combine the contributions from ambient, diffuse, specular, and spotlight
    vec3 result = ambient + diffuse + specular + spotlightColor ;
    color = vec4(result, 1.0);
}
