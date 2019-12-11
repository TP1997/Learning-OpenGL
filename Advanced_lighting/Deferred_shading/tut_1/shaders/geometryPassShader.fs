#version 460 core
//Geometry-pass shader. Collect all geometry information required and pass them to various colorbuffers
//(which content will be used in lighting-pass).

//Framebuffer's colorbuffer outputs.
layout(location=0) out vec3 gPosition;
layout(location=1) out vec3 gNormal;
layout(location=2) out vec4 gAlbedoSpec;

in VS_OUT{
    vec3 wsFragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main(){
    //Output world-space position to G-buffers gPosition-colorbuffer.
    gPosition=fs_in.wsFragPos;
    //Output fragment's normal vector to G-buffers gNormal-colorbuffer.
    gNormal=normalize(fs_in.Normal);
    //Output Diffuse color (RGB) + specular (A) intensity to G-buffers gAlbedoSpec-colorbuffer.
    gAlbedoSpec=vec4(texture(texture_diffuse1, fs_in.TexCoords).rgb, texture(texture_specular1, fs_in.TexCoords).r);

}

