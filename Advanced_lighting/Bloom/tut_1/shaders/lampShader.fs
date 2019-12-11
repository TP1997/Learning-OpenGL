#version 460 core
//Render lamps using high-dynamic-range floating point values. Rendering to the custom framebuffer.
//Output for multiple (2) colorbuffers.
layout(location=0) out vec4 FragColor;
layout(location=1) out vec4 BrightColor;

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform vec3 lightColor;

void main(){
    //Output to 'ordinary' colorbuffer.
    FragColor=vec4(lightColor, 1.0);

    //Output to 'brightness' colorbuffer.
    //Check whether fragment output is higher than threshold, if so output as brightness color.
    float brightness=dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness>1.0)
        BrightColor=FragColor;
    else
        BrightColor=vec4(0.0, 0.0, 0.0, 1.0);
}
