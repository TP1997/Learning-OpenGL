#version 460 core
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct DirLight{
	//vec3 direction;
    vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform DirLight light;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

void main(){
    vec3 color=texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal=normalize(fs_in.Normal);
//ambient component
    vec3 ambient=light.ambient*color;
//diffuse component
    vec3 lightDir=normalize(light.position-fs_in.FragPos);
    float diff=max(dot(lightDir, normal), 0.0);
    vec3 diffuse=diff*light.diffuse;
//specular component
    vec3 viewDir=normalize(viewPos-fs_in.FragPos);
    vec3 halfWayDir=normalize(lightDir+viewDir);
    float spec=pow(max(dot(normal, halfWayDir), 0.0), 64.0);
    vec3 specular=spec*light.specular;
//calculate shadow
    float shadow=ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
    vec3 lighting=(ambient+(1.0-shadow)*(diffuse+specular))*color;

    FragColor=vec4(lighting, 1.0);
}
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir){
    //Perform perspective divide. (clip space --> ndc, [-w, w] --> [-1, 1])
    //Meaningless since using an orthographic projection (w component remains untouched).
    vec3 projCoords=fragPosLightSpace.xyz/fragPosLightSpace.w;
    //Transform ndc, [-1, 1] --> [0, 1], because depth map ranges [0, 1] as well.
    projCoords=projCoords*0.5+0.5;
    //get closest depth value from the light's point of view
    float closestDepth=texture(shadowMap, projCoords.xy).r;
    //Current depth = fragment's z-value from light's perspective.
    float currentDepth=projCoords.z;
    //Check if current fragment is in shadow. 1.0 = in shadow.
    //Add bias to prevent shadow acne. Bias value is based on the surface angle towards the light.
    float bias=max(0.05*(1.0-dot(normal, lightDir)), 0.005);
    //If using front face culling, remove bias.
    //bias=0.0;
    float shadow=currentDepth>closestDepth+bias? 1.0 : 0.0;

    return shadow;
}
