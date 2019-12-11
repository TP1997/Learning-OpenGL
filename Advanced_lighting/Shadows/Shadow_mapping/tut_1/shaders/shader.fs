#version 460 core

out vec4 FragColor;
in VS_OUT{
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
} gs_in;

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
};

uniform sampler2D texture_;
uniform PointLight light;
uniform DirLight dLight;
uniform vec3 viewPos;
uniform bool blinn;

void main(){
//needed vectors
    vec3 normal=normalize(gs_in.Normal);
    vec3 color=texture(texture_, gs_in.TexCoords).rgb;
    vec3 viewDir=normalize(viewPos - gs_in.FragPos);
    vec3 lightDir=normalize(light.position - gs_in.FragPos);
    vec3 reflectDir=reflect(-lightDir, normal);
    vec3 halfWay=normalize(lightDir+viewDir);

//diffuse scalar
    float diff=max(dot(lightDir, normal), 0.0);

//specular scalar
    float spec=0.0;
    if(blinn){
        spec=pow(dot(normal, halfWay), 16.0);
    }
    else{
        spec=pow(max(dot(viewDir, reflectDir), 0), 8.0);
    }

//combine results
    vec3 ambient=light.ambient*color;
    vec3 diffuse=light.diffuse*diff*color;
    vec3 specular=light.specular*spec*color;

    FragColor=vec4(specular+ambient+diffuse, 1.0);
}

