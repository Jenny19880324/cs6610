#version 330
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D map_Kd; //Diffuse color texture map
void main(void){
    fragColor = vec4(0.1, 0, 0, 1) + texture(map_Kd, texCoord);
}
