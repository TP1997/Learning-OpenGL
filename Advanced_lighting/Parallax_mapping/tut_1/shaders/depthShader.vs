//Vertex shader (shadowmap)
//Responsible for transforming local-space vertices to world space and directing them to geometry shader.
#version 460 core
layout (location=0) in vec3 aPos;

uniform mat4 model;

void main(){
    //Transform vertex local position to world space.
    gl_Position=model*vec4(aPos, 1.0);
}
