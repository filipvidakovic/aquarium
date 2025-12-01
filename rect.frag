#version 330 core

in vec2 chTex;
out vec4 outCol;

uniform sampler2D uTex;
uniform sampler2D uTex1;
uniform bool useTex1;

void main()
{
    if (!useTex1) {
        outCol = texture(uTex, chTex);
    } else {
        vec4 c1 = texture(uTex, chTex);
        vec4 c2 = texture(uTex1, chTex);
        if (c2.a == 0) {
            outCol = c1;
        } else {
            outCol = c2;
        }
    }
} 