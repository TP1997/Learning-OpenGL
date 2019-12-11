#version 460 core
//Lighting-pass shader. Do actual lighting calculations in screen-space based on information stored in G-buffer's colorbuffers.
//Actual lighting calculations remain the same, but instead, information is readed from G-buffer.

out vec4 FragColor;
in VS_OUT{
    vec2 TexCoords;
} fs_in;

struct Light{
    vec3 position;
    float ambient;
    vec3 diffuse;
    vec3 specular;
    //Attenuation terms.
    float linear;
    float quadratic;
};
const int NR_LIGHTS=32;
uniform Light light[NR_LIGHTS];
uniform vec3 viewPos;
//Colorbuffers from G-buffer
uniform sampler2D gPosition;        //World-space fragment positions
uniform sampler2D gNormal;          //Fragment normals.
uniform sampler2D gAlbedoSpec;      //Fragment diffuse colors & specular components.

void main(){
    //Fetch geometry information from G-buffer.
    vec3 FragPos=texture(gPosition, fs_in.TexCoords).rgb;
    vec3 Normal=texture(gNormal, fs_in.TexCoords).rgb;
    vec3 Diffuse=texture(gAlbedoSpec, fs_in.TexCoords).rgb;
    float Specular=texture(gAlbedoSpec, fs_in.TexCoords).a;

    //Do lighting calculations as usual.
    vec3 viewDir=normalize(viewPos - FragPos);
    //Ambient.
    vec3 ambient=light[0].ambient*Diffuse;
    //Diffuse & Attenuation.
    vec3 lighting=vec3(0.0);
    for(int n=0; n<NR_LIGHTS; n++){
        //Diffuse.
        vec3 lightDir=normalize(light[n].position - FragPos);
        float diff=max(dot(lightDir, Normal), 0.0);
        vec3 diffuse=light[n].diffuse*diff*Diffuse;
        //Specular (blinn-phong).
        vec3 halfWay=normalize(lightDir + viewDir);
        float spec=pow(max(dot(Normal, halfWay), 0.0), 16.0);
        vec3 specular=light[n].specular*spec*Specular;
        //Attenuation.
        float distance=length(FragPos - light[n].position);
        float attenuation=1.0/(1.0 + light[n].linear*distance + light[n].quadratic*distance*distance);          //Quadratic attenuation.

        diffuse*=attenuation;
        specular*=attenuation;
        lighting+=diffuse + specular;
    }
    lighting+=ambient;

    FragColor=vec4(lighting, 1.0);
}
