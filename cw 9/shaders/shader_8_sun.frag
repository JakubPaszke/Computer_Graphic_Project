#version 430 core

uniform vec3 color;
uniform float exposition;
uniform sampler2D sunTexture;

in vec2 TexCoord;

out vec4 outColor;
void main()
{

    vec4 texColor = texture(sunTexture, TexCoord);

    vec3 finalColor = vec3(1.0) - exp(-color * exposition) * texColor.rgb;

    outColor = vec4(finalColor, 1.0);
}
