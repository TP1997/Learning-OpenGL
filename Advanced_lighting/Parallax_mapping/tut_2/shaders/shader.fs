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
//Steep parallax mapping

    //Number of depth layers.
    const float minLayers=8.0;
    const float maxLayers=32.0;
    //When viewing surface from an angle, take more samples to improve accuracy. Take less samples when viewing straight at the surface.
    const float numLayers=mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    //Size of layer.
    float layerDepth=1.0/numLayers;
    //Depth of current layer.
    float currentLayerDepth=0.0;
    //
    vec2 P=viewDir.xy/viewDir.z*height_scale;
    //
    vec2 deltaTexCoords=P/numLayers;
    //Get initial values
    vec2 currentTexCoords=texCoords;
    float currentDepthMapValue=texture(depthMap, currentTexCoords).r;

    while(currentLayerDepth<currentDepthMapValue){
        //Shift texture coordinates along direction of P.
        currentTexCoords-=deltaTexCoords;
        //Get depthmap value at current texture coordinates.
        currentDepthMapValue=texture(depthMap, currentTexCoords).r;
        //Get depth of next layer.
        currentLayerDepth+=layerDepth;
    }

//Parallax occulsion mapping
//Get even more accurate result for offsetted texture coordinate

    //Get texture coordinates before collision (order-swap-point between depthmap -and layer depth value)
    vec2 prevTexCoords=currentTexCoords+deltaTexCoords;
    //Get depth after and before collision
    float afterDepth=currentDepthMapValue-currentLayerDepth;
    float beforeDepth=texture(depthMap, prevTexCoords).r-currentLayerDepth+layerDepth;

    //Interpolation of texture coordinates
    float weight=afterDepth/(afterDepth-beforeDepth);
    vec2 finalTexCoords=prevTexCoords*weight+currentTexCoords*(1.0-weight);

    return finalTexCoords;
}
