#version 460 core
//Lighting-pass happens in screen-space so vertex shader is fairly simple. Just pass forward texture coordinates to
//perform lighting calculations in fragment shader (using information stored in G-buffer).
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoords;

out VS_OUT{
    vec2 TexCoords;
} vs_out;

void main(){
    //Output texture coordinates.
    vs_out.TexCoords=aTexCoords;

    gl_Position=vec4(aPos, 1.0);
}
