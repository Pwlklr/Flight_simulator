#version 330 core

// Uniforms
uniform sampler2D textureMap; // Texture map
uniform vec3 sunColor;        // Color of the sun
uniform float ambientStrength; // Strength of the ambient light
uniform vec3 viewPos;         // Position of the camera in world space

// Inputs from vertex shader
in vec3 fragPosition; // Position of the fragment in world space
in vec3 fragNormal;   // Normal vector of the fragment in world space
in vec2 fragTexCoord; // Texture coordinates
in vec3 lightDir;     // Direction to the light source

// Output to framebuffer
out vec4 color;

void main() {
    // Normalize input vectors
    vec3 norm = normalize(fragNormal);
    vec3 lightDirection = normalize(lightDir);
    vec3 viewDirection = normalize(viewPos - fragPosition);
    
    // Ambient lighting
    vec3 ambient = ambientStrength * sunColor;
    
    // Diffuse lighting
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * sunColor;
    
    // Specular lighting (Phong reflection model)
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * sunColor;
    
    // Combine results
    vec3 lighting = ambient + diffuse + specular;
    
    // Sample texture color
    vec4 texColor = texture(textureMap, fragTexCoord);
    
    // Combine texture color with lighting
    color = vec4(lighting, 1.0) * texColor;
}