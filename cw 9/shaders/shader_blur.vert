#version 430 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;


out vec2 TexCoords;

void main()
{
	gl_Position = vec4(vertexPosition, 1.0);
    TexCoords = vertexTexCoord;
}