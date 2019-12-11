#version 460 core
//Shader that blurs the image horizontally or vertically.

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;
uniform float gaussianWeight[5]={0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162};
void main(){
    //Get size of single textel
    vec2 texelSize=1.0/textureSize(image, 0);
    //Initalize result as current fragment's blurred color value.
    vec3 result=texture(image, TexCoords).rgb*gaussianWeight[0];
    if(horizontal){
        for(int n=1; n<5; ++n){                                                                     //Blur horizontally
            result+=texture(image, TexCoords+vec2(texelSize.x*n, 0.0)).rgb*gaussianWeight[n];       //To right
            result+=texture(image, TexCoords-vec2(texelSize.x*n, 0.0)).rgb*gaussianWeight[n];       //To left
        }
    }
    else{
        for(int n=1; n<5; ++n){                                                                     //Blur vertically
            result+=texture(image, TexCoords+vec2(0.0, texelSize.y*n)).rgb*gaussianWeight[n];       //To up
            result+=texture(image, TexCoords-vec2(0.0, texelSize.y*n)).rgb*gaussianWeight[n];       //To down
        }
    }

    FragColor=vec4(result, 1.0);
}
