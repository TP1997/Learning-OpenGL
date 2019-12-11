#version 460 core
//Geometry-pass shader (early stage). Do required transformations for geometry data and pass it forward to fragment-shader which
//collects data to several colorbuffers.

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;

out VS_OUT{
    vec3 wsFragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main(){
    //Output vertex position in world-space.
    vs_out.wsFragPos=vec3(model*vec4(aPos, 1.0));
    //Output normal vector.
    vs_out.Normal=transpose(inverse(mat3(model)))*aNormal;
    //Output texture coordinates.
    vs_out.TexCoords=aTexCoords;

    gl_Position=projection*view*model*vec4(aPos, 1.0);
}
