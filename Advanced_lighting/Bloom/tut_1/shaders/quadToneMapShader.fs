#version 460 core
//Map high-dynamic-range color values back to low-dynamic range. Rendering to the normal framebuffer.

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D hdrBuffer;        //Scene's hdr-texture
uniform sampler2D gaussianBlur;     //Scene's blurred brightness texture
uniform bool bloom;
uniform float exposure;
void main(){
    const float gamma=2.2;
    //Sample floating point colorbuffer.
    vec3 hdrColor=texture(hdrBuffer, TexCoords).rgb;
    vec3 bloomColor=texture(gaussianBlur, TexCoords).rgb;
    //Blend hdr & blurred texture back.
    if(bloom)
        hdrColor+=bloomColor;
    //Tone mapping.
    //Transform high-dynamic-range color values to low-dynamic-range ([0.0, 1.0]).
    vec3 result=vec3(1.0)-exp(-hdrColor*exposure);   //Exposure tone mapping.
    //Gamma correlation
    //result=pow(result, vec3(1.0/gamma));

    FragColor=vec4(result, 1.0);
}
