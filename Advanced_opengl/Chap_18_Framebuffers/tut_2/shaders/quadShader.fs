#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

vec4 normalColor();
vec4 inversedColor();
vec4 grayscale();

void main(){
//Normal
    //FragColor=normalColor();
//Inversion
    //FragColor=inversedColor();
//Grayscale
    FragColor=grayscale();

}

vec4 normalColor(){
    return texture(screenTexture, TexCoords);
}
vec4 inversedColor(){
    return vec4(vec3(1.0-texture(screenTexture, TexCoords)), 1.0);
}
vec4 grayscale(){
    vec4 temp=texture(screenTexture, TexCoords);
    //float average=(temp[0]+temp[1]+temp[2])/3.0;
    float average=temp[0]*0.2126+temp[1]*0.7152+temp[2]*0.0722;
    return vec4(average, average, average, 1.0);
}
