#version 330 core

//layout(location = 0) out float depth;
layout(location = 0) out vec4 fragColor;
void main(){
	//depth = gl_FragCoord.z;
	fragColor = vec4(1, 0, 0, 1);
}
