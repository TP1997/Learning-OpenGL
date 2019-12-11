#version 330 core
in vec3 Normal;
in vec3 fragPos;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

void main(){
	float ambientStrength=0.1;
	vec3 ambient = ambientStrength*lightColor;

	vec3 norm=Normal;//normalize(Normal);
	vec3 lightDir=normalize(lightPos-fragPos);

	float diff=max(dot(norm, lightDir), 0.0);
	vec3 diffuse=diff*lightColor;

	vec3 result = (ambient+diffuse)*objectColor;

	FragColor = vec4(result, 1.0);
}