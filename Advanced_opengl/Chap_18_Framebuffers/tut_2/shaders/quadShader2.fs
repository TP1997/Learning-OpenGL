#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset=1.0/300.0;
//Sharpen kernel
float skernel[9]={
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
};
//Blur kernel
float bkernel[9]={
    1.0/16, 2.0/16, 1.0/16,
    2.0/16, 4.0/16, 2.0/16,
    1.0/16, 2.0/16, 1.0/16
};
//Edge detection kernel
float edkernel[9]={
    1,  1, 1,
    1, -8, 1,
    1,  1, 1
};
void main(){
    vec2 offsets[9]={
        vec2(-offset, offset),
        vec2(0.0f,    offset),
        vec2(offset,  offset),
        vec2(-offset, 0.0f),
        vec2(0.0f,    0.0f),
        vec2(offset,  0.0f),
        vec2(-offset,-offset),
        vec2(0.0f,   -offset),
        vec2(offset, -offset)
    };

    vec3 sampleTex[9];
    for(int n=0; n<9; n++){
        sampleTex[n]=vec3(texture(screenTexture, TexCoords+offsets[n]));
    }
    vec3 col=vec3(0.0);
    for(int n=0; n<9; n++){
        col+=sampleTex[n]*edkernel[n];
    }
    FragColor=vec4(col, 1.0);
}
