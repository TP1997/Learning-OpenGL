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
uniform Light light;
uniform float height_scale;

uniform sampler2D diffuseTexture;       //Color texture
uniform sampler2D normalMap;            //Normal texture
uniform sampler2D depthMap;             //Depth values (parallax)

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
void main(){
//Parallax mapping operations
    //View direction in tangent-space
    vec3 tangentSpaceViewDir=normalize(fs_in.TangentSpaceViewPos-fs_in.TangentSpaceFragPos);
    //Get the displaced texture coordinates
    vec2 texCoords=ParallaxMapping(fs_in.TexCoords, tangentSpaceViewDir);
    //At the edges of the plane, the displaced texture coordinates could oversample outside the range [0,1]. Discard fragment if this happens.
    if(texCoords.x>1.0 || texCoords.x<0.0 || texCoords.y>1.0 || texCoords.y<0.0)
        discard;
//Usual lighting code
    vec3 color=texture(diffuseTexture, texCoords).rgb;
    //Obtain normal from normal map in range [0, 1]
    vec3 normal=texture(normalMap, texCoords).rgb;
    //[0, 1] --> [-1, 1]
    normal=normalize(normal*2-1);

//ambient component
    vec3 ambient=light.ambient;
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
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    //Sample height value from depth map.
    float height=texture(depthMap, texCoords).r;
    //Create vector parallel to viewDir and scaled by the height.
    //viewDir.z explanation:
    //viewDir is largely parallel to the surface --> z is close to 0.0 --> division returns larger vector p --> texture coordinates are offsetted at a larger space.
    vec2 p=viewDir.xy/viewDir.z*(height*height_scale);
    //Return texture coordinate with calculated offset.
    return texCoords-p;
}
