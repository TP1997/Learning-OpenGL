#version 460 core
out vec4 FragColor;

in VS_OUT{
    vec2 TexCoords
} fs_in;

uniform sampler2D screenTexture;
uniform float nPlane;
uniform float fPlane;

void main(){
    FragColor=texture(depthMap, fs_in.TexCoords).rgb;

}
