#version 330
in vec2 texCoord;

uniform sampler2D map_Kd;

out vec4 fragColor;

void main(void){
    fragColor = vec4(0.1, 0, 0, 1) + texture(map_Kd, texCoord);
    //fragColor = vec4(1.0, 0, 0, 1);
}

