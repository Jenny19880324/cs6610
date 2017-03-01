#version 330
layout(location = 0) in vec3 pos;

uniform mat4 modelViewProjection;

out vec3 textureDir;

void main(){
    textureDir = pos;
    gl_Position = modelViewProjection * vec4(pos, 1.0);
}
