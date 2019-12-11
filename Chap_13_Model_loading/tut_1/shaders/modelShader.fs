#version 330 core
#define POINT_LIGHTS 1

out vec4 FragColor;

struct Material{
	sampler2D texture_diffuse0;
	sampler2D texture_specular0;
	sampler2D texture_normal0;
	sampler2D texture_height0;
	float shininess;
};

struct DirLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight{
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
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
//light properties
uniform DirLight dirLight;
uniform PointLight pointLight[POINT_LIGHTS];
uniform SpotLight spotLight;

//function declarations
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main(){
	vec3 result;

	vec3 normal=normalize(Normal);
	vec3 viewDir=normalize(viewPos-FragPos);

//Directional light effect
	//result=calcDirLight(dirLight, normal, viewDir);
//Point light effect
	for(int n=0; n<POINT_LIGHTS; n++){
		result+=calcPointLight(pointLight[n], normal, FragPos, viewDir);
	}
//Spotlight effect
	result+=calcSpotLight(spotLight, normal, FragPos, viewDir);

	FragColor = vec4(result, 1.0);
}

//Direct light calculations
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	vec3 lightDir=normalize(-light.direction);
	vec3 reflectDir=reflect(-lightDir, normal);

//diffuse shading
	float diff=max(dot(normal, lightDir), 0.0);
//specular shading
	float spec=pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

//combine results
	vec3 ambient=light.ambient*texture(material.diffuse, TexCoords).rgb;
	vec3 diffuse=light.diffuse*diff*texture(material.diffuse, TexCoords).rgb;
	vec3 specular=light.specular*spec*texture(material.specular, TexCoords).rgb;
	return ambient+diffuse+specular;
}

//Point light calculations
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 lightDir=normalize(light.position-fragPos);
	vec3 reflectDir=reflect(-lightDir, normal);

//diffuse shading
	float diff=max(dot(normal, lightDir), 0.0);
//specular shading
	float spec=pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//attenuation
	float distance=length(light.position-fragPos);
	float attenuation=1.0/(light.constant + distance*light.linear + light.quadratic*distance*distance);

//combine results
	vec3 ambient = light.ambient*texture(material.diffuse, TexCoords).rgb;
	vec3 diffuse = light.diffuse*diff*texture(material.diffuse, TexCoords).rgb;
	vec3 specular= light.specular*spec*texture(material.specular, TexCoords).rgb;

	return attenuation*(ambient+diffuse+specular);
}

//SpotLight calculations
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 lightDir=normalize(light.position-fragPos);

	//diffuse shading
		float diff=max(dot(normal, lightDir), 0.0);
	//specular shading
		vec3 reflectDir=reflect(-lightDir, normal);
		float spec=pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	//attenuation
		float distance=length(light.position-fragPos);
		float attenuation=1.0/(light.constant + distance*light.linear + light.quadratic*distance*distance);
	//intensity
		float theta=dot(lightDir, normalize(-light.spotDir));
		float intensity=clamp((theta-light.outerCutOff)/(light.innerCutOff-light.outerCutOff), 0.0, 1.0);

	//combine results
		vec3 ambient=light.ambient*vec3(texture(material.diffuse, TexCoords));
		vec3 diffuse=light.diffuse*diff*vec3(texture(material.diffuse, TexCoords));
		vec3 specular=light.specular*spec*vec3(texture(material.specular, TexCoords));
		ambient*=attenuation*intensity;
		diffuse*=attenuation*intensity;
		specular*=attenuation*intensity;

		return ambient+diffuse+specular;	
}