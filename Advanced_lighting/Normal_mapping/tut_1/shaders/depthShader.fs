//Fragment shader (shadowmap)
//Responsible for calculating (linear) depth values as the linear distance between each fragment position and the light source's position.
//(Not neccessary to calculate on our own).
#version 460 core
in vec4 FragPos;            //Input from geometry shader.

uniform vec3 lightPos;
uniform float far_plane;

void main(){
    //Get distance between fragment and light source.
    float lightDistance=length(FragPos.xyz-lightPos);
    //Map to [0, 1] range
    lightDistance/=far_plane;
    //Write this as modified depth.
    gl_FragDepth=lightDistance;
}
