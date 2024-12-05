#version 410 core

// Structure to represent light (Directional, Point, and Spotlight)
struct Light {
    vec3 position;   // Position of light
    vec3 color;      // Color of light
    // float intensity; // Intensity of light
};

// Input from vertex shader
in vec3 fragNormal;       // Normal passed from vertex shader
in vec2 fragTexCoord;     // Texture coordinates passed from vertex shader
in vec3 fragPosition;     // World-space position of the fragment
out vec4 fragColor;       // Final output color

// Uniforms for material textures
uniform sampler2D diffuseTexture;        // Base color texture
uniform sampler2D normalTexture;         // Normal map texture
uniform sampler2D metallicRoughnessTexture; // Combined metallic-roughness map
uniform sampler2D occlusionTexture;      // Ambient occlusion texture
uniform sampler2D emissiveTexture;       // Emissive texture

// Material properties
uniform vec4 diffuseColor;    // Fallback diffuse color
uniform vec3 emissiveColor;   // Fallback emissive color
uniform float metallic;       // Fallback metallic value
uniform float roughness;      // Fallback roughness value

uniform vec3 cameraPosition;

uniform Light light;

vec3 srgbToLinear(vec3 color) {
    return mix(pow(color, vec3(2.4)), color / 12.92, lessThanEqual(color, vec3(0.04045)));
}

vec3 linearToSrgb(vec3 color) {
    return mix(12.92 * color, (1.055 * pow(color, vec3(1.0 / 2.4))) - 0.055, lessThanEqual(color, vec3(0.0031308)));
}

void main() {
    // Default values for material properties
    vec3 baseColor = diffuseColor.rgb;  // Default to uniform diffuse color
    vec3 normalMap = vec3(0.0, 0.0, 1.0); // Default normal (flat)
    float finalMetallic = metallic;     // Default metallic
    float finalRoughness = roughness;   // Default roughness
    vec3 emissive = emissiveColor;      // Default emissive color

    // Fetch and use textures if available
    if (textureSize(diffuseTexture, 0).x > 0) {
        baseColor = texture(diffuseTexture, fragTexCoord).rgb;
    }

    if (textureSize(normalTexture, 0).x > 0) {
        normalMap = texture(normalTexture, fragTexCoord).rgb;
        normalMap = normalize(normalMap * 2.0 - 1.0); // Convert from [0,1] to [-1,1]
    }

    if (textureSize(metallicRoughnessTexture, 0).x > 0) {
        vec2 metallicRoughness = texture(metallicRoughnessTexture, fragTexCoord).rg;
        finalMetallic = metallicRoughness.r;
        finalRoughness = metallicRoughness.g;
    }

    if (textureSize(emissiveTexture, 0).x > 0) {
        emissive = texture(emissiveTexture, fragTexCoord).rgb;
    }

    // Ambient occlusion (optional)
    float occlusion = 1.0;
    if (textureSize(occlusionTexture, 0).x > 0) {
        occlusion = texture(occlusionTexture, fragTexCoord).r;
    }

	float ambientStrength = 0.1846;
    vec3 ambient = ambientStrength * vec3(1.0);

	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(light.position - fragPosition);  

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light.color;

	float specularStrength = 0.4;
	float specularExponent = 24.0; // Soft, subtle specular highlight for skin
	vec3 viewDir = normalize(cameraPosition - fragPosition);
	vec3 reflectDir = reflect(-lightDir, norm);  

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularExponent);
	vec3 specular = specularStrength * spec * light.color;  

	vec3 result = (ambient + diffuse + specular) * baseColor;
    fragColor = vec4(result, 1.0);
}