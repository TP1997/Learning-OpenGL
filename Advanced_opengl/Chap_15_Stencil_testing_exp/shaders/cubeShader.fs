#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_;
float near=0.1;
float far=100;

float linearizeDepth(float depth){
	float zn=depth*2.0-1.0;
	float linearDepth=(2.0*near*far)/(far+near-zn*(far-near));
	return linearDepth;
}

void main(){

	FragColor = texture(texture_, TexCoords);
	//float depth=linearizeDepth(gl_FragCoord.z)/far;
	//FragColor=vec4(vec3(depth), 1.0);
}

