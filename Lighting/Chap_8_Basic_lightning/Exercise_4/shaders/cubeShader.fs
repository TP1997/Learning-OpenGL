#version 330 core
in vec3 Normal;
in vec3 fragColor;

out vec4 FragColor;

uniform vec3 objectColor;

void main(){

	FragColor = vec4(fragColor*objectColor, 1.0);
}