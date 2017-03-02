#version 330
uniform sampler2D map_Kd;

out vec4 fragColor;

void main(void){
    vec2 texCoord;
    texCoord.x = gl_FragCoord.x / 800;
    texCoord.y = 1.0 - gl_FragCoord.y / 600;
    fragColor = texture(map_Kd, texCoord);
}

