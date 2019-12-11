#version 460 core
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    //vec4 FragPosLightSpace;
} fs_in;

struct DirLight{
	//vec3 direction;
    vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform sampler2D diffuseTexture;       //Normal texture
uniform samplerCube depthMap;           //(closest) Depth values from light's perspective

uniform DirLight light;
uniform vec3 viewPos;

uniform float far_plane;

vec3 sampleOffsetDirections[20]={vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
                                 vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
                                 vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
                                 vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
                                 vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)};
float ShadowCalculation(vec3 fragPos);
float calculatePCF(vec3 fragPos, vec3 fragToLight, float currentDepth);
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
    float shadow=ShadowCalculation(fs_in.FragPos);
    vec3 lighting=(ambient+(1.0-shadow)*(diffuse+specular))*color;

    FragColor=vec4(lighting, 1.0);
}
float ShadowCalculation(vec3 fragPos){
    //Take a direction vector between current fragment and light's position. Use this vector to sample the cubemap.
    vec3 fragToLight=fragPos-light.position;
    float closestDepth=texture(depthMap, fragToLight).r;
    //closestDepth-value is in range [0, 1]. Transform [0, 1] --> [0, far_plane]
    closestDepth*=far_plane;
    //Retrieve current depth value by taking a length of fragToLight-vector.
    float currentDepth=length(fragToLight);

    //Check if fragment is in shadow.
    float bias=0.05;
    //bias=max(0.05*(1.0-dot(normal, lightDir)), 0.005);
    //float shadow=currentDepth>closestDepth+bias ? 1.0 : 0.0;
    //Or calculate PCF for smoother shadows
    float shadow=calculatePCF(fragPos, fragToLight, currentDepth);
    return shadow;
}
float calculatePCF(vec3 fragPos, vec3 fragToLight, float currentDepth){
    float shadow=0.0;
    float bias=0.15;
    int samples=20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius=(1.0+(viewDistance/far_plane))/25.0;
    for(int n=0; n<samples; n++){
        float closestDepth=texture(depthMap, fragToLight+sampleOffsetDirections[n]*diskRadius).r;
        closestDepth*=far_plane;
        shadow+=currentDepth>closestDepth+bias ? 1.0 : 0.0;
    }
    shadow/=float(samples);
    return shadow;

}
