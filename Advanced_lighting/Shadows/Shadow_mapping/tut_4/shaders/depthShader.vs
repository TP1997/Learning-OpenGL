#version 460 core
//Shader for creating depth buffer from light's perspective.

layout (location=0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main(){
    //Transform vertex local position to light space.
    gl_Position=lightSpaceMatrix*model*vec4(aPos, 1.0);
}
