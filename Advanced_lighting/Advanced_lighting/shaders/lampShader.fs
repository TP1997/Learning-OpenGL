#version 330
out vec4 FragColor;

in VS_OUT{
   vec3 FragColor;
} gs_in;

void main(){
    FragColor=vec4(gs_in.FragColor, 1.0);
}
