#version 440 core

layout (location=0) in vec3 inVert;
layout (location=1) in vec3 inNormal;
layout (location=2) in vec3 inUV;
uniform mat4 MVP; //Model View Project - product of 3 matrices
void main()
{
    gl_Position=MVP*vec4(inVert,1.0);
}
