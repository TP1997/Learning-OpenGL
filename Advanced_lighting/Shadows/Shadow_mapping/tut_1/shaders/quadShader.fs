#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float nPlane;
uniform float fPlane;

void main(){
    float depthVal=texture(depthMap, TexCoords).r;
    FragColor=vec4(vec3(depthVal), 1.0);
}
