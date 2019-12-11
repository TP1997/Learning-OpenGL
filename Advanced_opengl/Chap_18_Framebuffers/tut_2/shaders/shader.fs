#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_;

void main(){
    FragColor=texture(texture_, TexCoords);
}

