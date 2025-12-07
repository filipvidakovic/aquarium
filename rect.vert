#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;
out vec2 chTex;

uniform float uX;
uniform float uY;
uniform float uS; // skaliranje geometrijskog tela množenjem
uniform float uFlipX; 

void main()
{
    gl_Position = vec4(inPos.x * uS + uX, inPos.y * uS + uY, 0.0, 1.0);
    chTex = inTex;
    if (uFlipX < 0.0) {
        chTex.x = 1.0 - chTex.x;
    }
}