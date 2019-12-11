#version 460 core
//Shader that blurs the image horizontally or vertically.
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoords;

out vec2 TexCoords;

void main(){
    TexCoords=aTexCoords;
    gl_Position=vec4(aPos, 1.0);
}
