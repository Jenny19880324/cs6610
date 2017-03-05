#version 330
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 inputNormal;

out vec3 normalInterp;
out vec3 vertPos;
out vec3 lightDir;
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
	
	mat4 Ms = mat4(vec4(0.5, 0, 0, 0),
					vec4(0, 0.5, 0, 0),
					vec4(0, 0, 0.5, 0),
					vec4(0.5, 0.5, 0.5, 1));
	shadowCoord = Ms * lightModelViewProjection * vec4(pos, 1.0);
	
	const vec3 lightPosition = vec3(20, 40, 20);
	vec4 lightDir4 = vec4(lightPosition - pos, 0.0);
	lightDir4 = modelView * lightDir4;
	lightDir = normalize(lightDir4).xyz;
}
