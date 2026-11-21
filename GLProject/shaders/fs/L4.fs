#version 460 core
out vec4 FragColor;

in vec2 texCoord;

uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;
uniform bool useTexture;

void main()
{
    vec4 surfaceColor = useTexture ? texture(texture_diffuse1, texCoord) : vec4(1.0);
    FragColor = vec4(surfaceColor.rgb * objectColor, surfaceColor.a);
}