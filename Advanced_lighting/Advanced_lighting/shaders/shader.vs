#version 460 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec2 aTexCoords;
layout (location=2) in vec3 aNormal;

out VS_OUT{
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
} gs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(){
	gl_Position=projection*view*model*vec4(aPos, 1.0f);
	gs_out.TexCoords=aTexCoords;
	gs_out.Normal=mat3(transpose(inverse(model)))*aNormal;
	gs_out.FragPos=vec3(model*vec4(aPos, 1.0));
}
