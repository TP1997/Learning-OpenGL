#version 460 core
//Render to custom framebuffer's colorbuffer.
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;

out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    //Output vertex position in world-space.
    vs_out.FragPos=vec3(model*vec4(aPos, 1.0));
    //Output texture coordinates.
    vs_out.TexCoords=aTexCoords;
    //Output inverse of normal vector.
    vs_out.Normal=transpose(inverse(mat3(model)))*(-aNormal);

    gl_Position=projection*view*model*vec4(aPos, 1.0);
}
