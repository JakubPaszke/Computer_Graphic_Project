#version 430 core

uniform vec3 lightPos;
uniform sampler2D colorTexture;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;
void main()
{
	vec3 lightDir = normalize(lightPos-worldPos);
	outColor = texture2D(colorTexture, vecTex);
}
