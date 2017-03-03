#version 330
layout(location = 0) in vec3 pos;

uniform mat4 modelViewProjection;

void main(){
    gl_Position = modelViewProjection * vec4(pos, 1.0);
}
