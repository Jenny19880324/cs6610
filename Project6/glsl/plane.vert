#version 330
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inputTexCoord;

out vec2 texCoord;

uniform mat4 modelViewProjection;

void main(){
    gl_Position = modelViewProjection * vec4(pos, 1.0);
    texCoord = inputTexCoord;
}
