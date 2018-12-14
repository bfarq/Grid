#version 410 core

layout (location=0) in vec3 inVert;
layout (location=1) in vec4 inColour;
//layout (location=2) in vec3 inUV;
out vec4 colour;
uniform mat4 MVP; //Model View Project - product of 3 matrices
void main()
{
    colour=inColour;
    gl_Position=MVP*vec4(inVert,1.0);
}
