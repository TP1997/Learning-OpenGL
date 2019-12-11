#version 330 core
out vec4 FragColor;

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light{
	vec3 spotDir;
	vec3 position;
	float innerCutOff;
	float outerCutOff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

in vec3 Normal;
in vec3 fragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main(){
	vec3 result;

	vec3 lightDir=normalize(light.position-fragPos);
	float theta=dot(lightDir, normalize(-light.spotDir));
	float epsilon=light.innerCutOff-light.outerCutOff;
	float intensity=clamp((theta-light.outerCutOff)/epsilon, 0.0, 1.0);

	if(theta>light.outerCutOff){
		//Effects:

		//ambient
		vec3 ambient = light.ambient*texture(material.diffuse, TexCoords).rgb;

		//diffuse
		vec3 norm=normalize(Normal);
		float diff=max(dot(norm, lightDir), 0.0);
		vec3 diffuse=light.diffuse*diff*texture(material.diffuse, TexCoords).rgb;

		//specular
		vec3 viewDir=normalize(viewPos-fragPos);
		vec3 reflectDir=reflect(-lightDir, norm);
		float spec=pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		vec3 specular=light.specular*spec*texture(material.specular, TexCoords).rgb;

		//attenuation
		float distance=length(light.position-fragPos);
		float attenuation=1.0/(light.constant + distance*(light.linear + 0.07*light.quadratic));
		//diffuse*=intensity;
		//specular*=intensity;
		result = attenuation*(ambient+intensity*(diffuse+specular));
	}
	else{
		result=light.ambient*texture(material.diffuse, TexCoords).rgb;
	}
	
	FragColor = vec4(result, 1.0);
}