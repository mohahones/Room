#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 texCoord;
out vec3 worldNormal;
out vec3 worldPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 textureScale;

void main()
{
    vec4 position = model * vec4(aPos, 1.0);
    gl_Position = projection * view * position;
    texCoord = aTexCoord * textureScale;
    worldNormal = mat3(transpose(inverse(model))) * aNormal;
    worldPosition = position.xyz;
}