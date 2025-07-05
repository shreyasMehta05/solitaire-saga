#version 330

layout (location = 0) in vec3 Position;

uniform mat4 gWorld;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    // Transform the vertex position
    gl_Position = gWorld * vec4(Position, 1.0);

    
    // Just pass the local position (relative to marble center)
    // for lighting calculations in the fragment shader
    FragPos = Position;
    
    // Calculate normal for lighting (assuming sphere)
    // For a sphere, the normal is just the normalized position from center
    Normal = normalize(Position);
}
