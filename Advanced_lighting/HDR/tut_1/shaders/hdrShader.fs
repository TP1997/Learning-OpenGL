#version 460 core
//Map high-dynamic-range color values back to low-dynamic range. Rendering to the normal framebuffer.

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;
void main(){
    const float gamma=2.2;
    //Sample floating point colorbuffer.
    vec3 hdrColor=texture(hdrBuffer, TexCoords).rgb;
    vec3 result=hdrColor;
    if(hdr){
        //Tone mapping.
        //Transform high-dynamic-range color values to low-dynamic-range ([0.0, 1.0]).
        result=vec3(1.0)-exp(-hdrColor*exposure);   //Exposure tone mapping.
        //result=hdrColor/(hdrColor+vec3(1.0));       //Reinhard tone mapping
    }
    //Gamma correlation.
    result=pow(result, vec3(1.0/gamma));

    FragColor=vec4(result, 1.0);
}
