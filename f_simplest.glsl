#version 330 core

// Uniforms
uniform sampler2D textureMap; // Texture map
uniform vec3 sunColor;        // Color of the sun
uniform vec3 secondLightColor; // Color of the second light source
uniform float ambientStrength; // Strength of the ambient light
uniform float secondLightIntensity; // Intensity of the second light
uniform vec3 viewPos;         // Position of the camera in world space

// Inputs from vertex shader
in vec3 fragPosition; // Position of the fragment in world space
in vec3 fragNormal;   // Normal vector of the fragment in world space
in vec2 fragTexCoord; // Texture coordinates
in vec3 lightDir;     // Direction to the light source
in vec3 secondLightDir; // Direction to the second light source


// Output to framebuffer
out vec4 color;

void main() {
    // Normalize input vectors
    vec3 norm = normalize(fragNormal);
    vec3 lightDirection = normalize(lightDir);
    vec3 secondLightDirection = normalize(secondLightDir);
    vec3 viewDirection = normalize(viewPos - fragPosition);

    // Sample texture color
    vec4 texColor = texture(textureMap, fragTexCoord);
    
    // Ambient lighting
    vec3 ambient = ambientStrength * sunColor;
    
    // Diffuse lighting for both light sources
    float diff = max(dot(norm, lightDirection), 0.0);
    float secondDiff = max(dot(norm, secondLightDirection), 0.0);
    
    // Apply attenuation to the second light based on distance
    float distance = length(secondLightDir);
    float attenuation = 1.0 / distance; // distance shadering from the secondary light source
    
    vec3 diffuse = diff * sunColor + (secondDiff * secondLightColor * secondLightIntensity * attenuation);
    
    // Specular lighting (Phong reflection model) for both light sources
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDirection, norm);
    vec3 secondReflectDir = reflect(-secondLightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 32);
    float secondSpec = pow(max(dot(viewDirection, secondReflectDir), 0.0), 32);
    vec3 specular = specularStrength * (spec * sunColor + (secondSpec * secondLightColor * secondLightIntensity * attenuation));
    
    // Combine results
    vec3 lighting = ambient + diffuse + specular;
    
    // Combine texture color with lighting
    color = vec4(lighting, 1.0) * texColor;
}