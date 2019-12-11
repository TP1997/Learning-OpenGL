#version 330 core
layout (location=0) in vec3 aPos;

out VS_OUT{
    vec3 FragColor;
} gs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightColor;

void main(){
	gl_Position=projection*view*model*vec4(aPos, 1.0f);
	gs_out.FragColor=lightColor;
}
