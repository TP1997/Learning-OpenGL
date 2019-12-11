#version 460 core
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentSpaceLightPos;
    vec3 TangentSpaceViewPos;
    vec3 TangentSpaceFragPos;
} fs_in;

struct Light{
	//vec3 direction;
    vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform sampler2D diffuseTexture;       //Color texture
uniform sampler2D normalMap;            //Normal texture

uniform Light light;

void main(){
    vec3 color=texture(diffuseTexture, fs_in.TexCoords).rgb;
    //Obtain normal from normal map in range [0, 1]
    vec3 normal=texture(normalMap, fs_in.TexCoords).rgb;
    //[0, 1] --> [-1, 1]
    normal=normalize(normal*2-1);

//ambient component
    vec3 ambient=light.ambient*color;
//diffuse component
    vec3 lightDir=normalize(fs_in.TangentSpaceLightPos-fs_in.TangentSpaceFragPos);
    float diff=max(dot(lightDir, normal), 0.0);
    vec3 diffuse=diff*light.diffuse;
//specular component
    vec3 viewDir=normalize(fs_in.TangentSpaceViewPos-fs_in.TangentSpaceFragPos);
    vec3 halfWayDir=normalize(lightDir+viewDir);
    float spec=pow(max(dot(normal, halfWayDir), 0.0), 64.0);
    vec3 specular=spec*light.specular;

    vec3 result=(ambient+diffuse+specular)*color;

    FragColor=vec4(result, 1.0);
}

