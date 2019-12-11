#version 460 core
layout(points) in;
layout(triangle_strip, max_vertices=5) out;

out vec3 fColor;
in VS_OUT{
    vec3 color;
} gs_in[];

void build_house(vec4 pos);
void main(){
    build_house(gl_in[0].gl_Position);
}

void build_house(vec4 pos){
    fColor=gs_in[0].color;

    gl_Position=pos+vec4(-0.2, -0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position=pos+vec4(0.2, -0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position=pos+vec4(-0.2, 0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position=pos+vec4(0.2, 0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position=pos+vec4(0.0, 0.4, 0.0, 0.0);
    fColor=vec3(1.0, 1.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
