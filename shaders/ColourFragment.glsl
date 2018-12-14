#version 440 core
layout (location=0) out vec4 fragColour;
uniform vec4 vertColour;

void main()
{
    fragColour=vertColour;
}
