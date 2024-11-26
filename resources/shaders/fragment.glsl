#version 410 core

in vec3 fragNormal;       // Normal passed from vertex shader
in vec2 fragTexCoord;     // Texture coordinates passed from vertex shader
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

void main() {
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

    // Lighting calculation (simple example)
    vec3 lightDir = normalize(vec3(0.5, 0.5, -1.0)); // Directional light
    vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0));   // View direction (camera)
    vec3 halfDir = normalize(lightDir + viewDir);    // Halfway vector

    float diffuse = max(dot(normalMap, lightDir), 0.0);
    float specular = pow(max(dot(normalMap, halfDir), 0.0), 32.0);

    // Combine results using metallic-roughness
    vec3 reflectedColor = mix(vec3(0.04), baseColor, finalMetallic);
    vec3 lighting = occlusion * diffuse * baseColor + specular * reflectedColor;

    // Add emissive color
    vec3 finalColor = lighting + emissive;

    fragColor = vec4(finalColor, diffuseColor.a);
}