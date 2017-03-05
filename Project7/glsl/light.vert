#version 330
layout(location = 0) in vec3 pos;

uniform mat4 modelViewProjection;
uniform mat4 lightRotation;

void main(){
	const vec3 lightPosition = vec3(20, 20, 20);
	gl_Position = modelViewProjection * lightRotation * vec4(pos + lightPosition, 1.0);
}
