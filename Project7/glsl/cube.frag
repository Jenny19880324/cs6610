#version 330
in vec3 textureDir;
uniform samplerCube cubemap;

layout(location = 0) out vec4 fragColor;

void main(void){
    fragColor =  texture(cubemap, textureDir);
}
