#version 330
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 inputNormal;
layout(location = 2) in vec2 inputTexCoord;

out vec3 normalInterp;
out vec3 vertPos;
out vec4 shadowCoord;

uniform mat4 modelViewProjection;
uniform mat4 modelView;
uniform mat4 normalTransform;
uniform mat4 lightModelViewProjection;

void main(){
    gl_Position = modelViewProjection * vec4(pos, 1.0);
    vec4 vertPos4 = modelView * vec4(pos, 1.0);
    vertPos = vec3(vertPos4) / vertPos4.w;
    normalInterp = (normalTransform * vec4(inputNormal, 0.0)).xyz;
	shadowCoord = lightModelViewProjection * vec4(pos, 1.0);
}
