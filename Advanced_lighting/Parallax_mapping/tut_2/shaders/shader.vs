#version 460 core
//Shader for rendering the scene.
//Style 2.
//Take inverse of TBN-matrix and transform all relevant vectors from world-space to tangent-space.
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aTexCoords;
layout (location=3) in vec3 aTangent;
layout (location=4) in vec3 aBitangent;

out VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentSpaceLightPos;
    vec3 TangentSpaceViewPos;
    vec3 TangentSpaceFragPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main(){
    vs_out.FragPos=vec3(model*vec4(aPos, 1.0));                             //World space vertex position
    vs_out.TexCoords=aTexCoords;
    //Transform TBN basis vectors to the world-space coordinate system
    vec3 T=normalize(vec3(model*vec4(aTangent, 0.0)));
    vec3 B=normalize(vec3(model*vec4(aBitangent, 0.0)));
    vec3 N=normalize(vec3(model*vec4(aNormal, 0.0)));
    //Get an inverse of TBN-matrix (see description)
    mat3 TBN=transpose(mat3(T, B, N));
    //Transform all relevant vectors in world space to tangent space
    vs_out.TangentSpaceLightPos=TBN*lightPos;
    vs_out.TangentSpaceViewPos=TBN*viewPos;
    vs_out.TangentSpaceFragPos=TBN*vec3(model*vec4(aPos, 1.0));

    gl_Position=projection*view*model*vec4(aPos, 1.0);                      //Camera (normal) space vertex position

}
