#version 460 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 worldNormal;
in vec3 worldPosition;

uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;
uniform bool useTexture;
uniform bool useLighting;
uniform vec3 viewPosition;
uniform float materialShininess;
uniform float materialSpecularStrength;
uniform float materialMetallic;

void main()
{
    vec4 surfaceColor = useTexture ? texture(texture_diffuse1, texCoord) : vec4(1.0);
    vec3 color = surfaceColor.rgb * objectColor;
    if (useLighting)
    {
        vec3 normal = normalize(worldNormal);
        vec3 viewDirection = normalize(viewPosition - worldPosition);
        vec3 keyDirection = normalize(vec3(-0.4, 0.85, 0.45));
        vec3 fillDirection = normalize(vec3(0.65, 0.3, -0.55));
        float keyLight = max(dot(normal, keyDirection), 0.0);
        float fillLight = max(dot(normal, fillDirection), 0.0);
        vec3 halfDirection = normalize(keyDirection + viewDirection);
        float specular = pow(max(dot(normal, halfDirection), 0.0),
            materialShininess) * materialSpecularStrength;
        float rim = pow(1.0 - max(dot(normal, viewDirection), 0.0), 3.0) * 0.08;
        vec3 highlight = mix(vec3(specular), color * specular, materialMetallic);
        color = color * (0.24 + 0.58 * keyLight + 0.18 * fillLight + rim) + highlight;
    }
    FragColor = vec4(color, surfaceColor.a);
}