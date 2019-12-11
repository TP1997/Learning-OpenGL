//Geometry shader (shadowmap)
//Responsible for transforming all world space vertices (from vertex shader) to 6 different light spaces.
//Takes as input a triangle
//Output total of 6 triangles (6*3 vertices)
#version 460 core
layout(triangles) in;                   //Takes as input 3 triangle vertices
layout(triangle_strip, max_vertices=18) out;

out vec4 FragPos;

uniform mat4 lightSpaceMatrices[6];

void main(){
    for(int face=0; face<6; ++face){                    //Iterate over 6 faces
        gl_Layer=face;                                  //Built-in variable that specifies to wich face we render. Works only when we have cubemap texture attached to the active framebuffer.
        for(int n=0; n<3; ++n){                         //For each triangle's vertices.
            FragPos=gl_in[n].gl_Position;               //Send world-space vertex to fragment shader without modification (used in depth value calculation).
            gl_Position=lightSpaceMatrices[face]*FragPos;  //Transform world-space vertex to the relevant light space
            EmitVertex();
        }
        EndPrimitive();
    }
}

