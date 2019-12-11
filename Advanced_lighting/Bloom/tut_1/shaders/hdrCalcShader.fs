#version 460 core
//Calculate lighting using high-dynamic-range floating point values. Rendering to the custom framebuffer.
//Output for multiple (2) colorbuffers.
layout(location=0) out vec4 FragColor;
layout(location=1) out vec4 BrightColor;
in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Light{
    vec3 position;

	float ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform Light light[4];

uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

void main(){
    vec3 color=texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal=normalize(fs_in.Normal);
    //ambient
    vec3 ambient=light[0].ambient*color;
    //Diffuse & Attenuation
    vec3 lighting=vec3(0.0);
    for(int n=0; n<4; n++){
        //diffuse
        vec3 lightDir=normalize(light[n].position-fs_in.FragPos);
        float diff=max(dot(lightDir, normal), 0.0);
        vec3 diffuse=light[n].diffuse*diff*color;
        vec3 result=diffuse;
        //Attenuation
        float distance=length(fs_in.FragPos-light[n].position);
        float attenuation=1.0/(distance*distance);
        result*=attenuation;

        lighting+=result;
    }
    //Output to 'ordinary' colorbuffer.
    vec3 result=ambient+lighting;
    //FragColor=vec4(ambient+lighting, 1.0);

    //Output to 'brightness' colorbuffer.
    //Check whether fragment output is higher than threshold, if so output as brightness color.
    float brightness=dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness>5.0)
        BrightColor=vec4(result, 1.0);
    else
        BrightColor=vec4(0.0, 0.0, 0.0, 1.0);

    FragColor=vec4(result, 1.0);
}
