#version 330 core
in vec3 Normal;
in vec3 fragPos;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lampPos;
uniform vec3 viewPos;

void main(){
	float ambientStrength=0.2;
	float specularStrength=0.5;

	vec3 ambient = ambientStrength*lightColor;

	vec3 norm=Normal;//normalize(Normal);
	vec3 lightDir=normalize(lampPos-fragPos);
	float diff=max(dot(norm, lightDir), 0.0);
	vec3 diffuse=diff*lightColor;

	vec3 viewDir=normalize(viewPos-fragPos);
	vec3 reflectDir=reflect(-lightDir, norm);
	float spec=pow(max(dot(viewDir, reflectDir), 0.0), 256);
	vec3 specular=specularStrength*spec*lightColor;

	vec3 result = (ambient+diffuse+specular)*objectColor;

	FragColor = vec4(result, 1.0);
}