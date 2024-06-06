#version 330 core

// Uniforms
uniform mat4 P;     // Projection matrix
uniform mat4 V;     // View matrix
uniform mat4 M;     // Model matrix
uniform vec4 sunPosition; // Position of the sun in world space
uniform vec4 secondLightPosition; // Position of the second light source in world space

// Attributes
in vec4 vertex;    // Vertex coordinates in model space
in vec4 normal;    // Normal vector in model space
in vec2 texCoord0;  // Texture coordinates

// Interpolated outputs to fragment shader
out vec3 fragPosition; // Position of the fragment in world space
out vec3 fragNormal;   // Normal vector of the fragment in world space
out vec2 fragTexCoord; // Texture coordinates
out vec3 lightDir;     // Direction to the light source
out vec3 secondLightDir;     // Direction to the light source

void main() {
    // Transform vertex position to world space
    vec4 worldPosition = M * vertex;
    fragPosition = vec3(worldPosition);
    
    // Transform normal vector to world space
    fragNormal = mat3(transpose(inverse(M))) * vec3(normal);
    
    // Pass texture coordinates to fragment shader
    fragTexCoord = texCoord0;
    
    // Calculate light direction
    lightDir = vec3(sunPosition) - fragPosition;
    secondLightDir = vec3(secondLightPosition) - fragPosition;

    
    // Transform vertex position to clip space
    gl_Position = P * V * worldPosition;
}