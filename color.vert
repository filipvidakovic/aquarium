#version 330 core

layout(location = 0) in vec2 inPos;
uniform float uX;
uniform float uY;
uniform float uS;

void main()
{
    gl_Position = vec4(inPos.x * uS + uX, inPos.y * uS + uY, 0.0, 1.0);
}