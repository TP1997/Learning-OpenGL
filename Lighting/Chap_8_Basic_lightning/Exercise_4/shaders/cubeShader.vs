#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;

out vec3 Normal;
out vec3 fragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lampPos;
uniform vec3 viewPos;


void main(){
	gl_Position=projection*view*model*vec4(aPos, 1.0f);
	Normal=aNormal;

	float ambientStrength=0.1;
	float specularStrength=1.0;
	vec3 fragPos=vec3(model*vec4(aPos, 1.0));

	vec3 ambient = ambientStrength*lightColor;

	vec3 norm=normalize(Normal);
	vec3 lightDir=normalize(lampPos-fragPos);
	float diff=max(dot(norm, lightDir), 0.0);
	vec3 diffuse=diff*lightColor;

	vec3 viewDir=normalize(viewPos-fragPos);
	vec3 reflectDir=reflect(-lightDir, norm);
	float spec=pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular=specularStrength*spec*lightColor;

	fragColor= ambient+diffuse+specular;

}
