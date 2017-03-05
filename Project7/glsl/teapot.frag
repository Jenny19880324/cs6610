#version 330
in vec3 normalInterp;
in vec3 vertPos;
in vec3 lightDir;
in vec4 shadowCoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2DShadow map_Shadow; //depth texture map

void main(void){
	const float Ns = 18.0;
	const vec3 Ka = vec3(0.2, 0.1, 0.1);
	const vec3 Kd = vec3(0.6, 0.3, 0.3);
	const vec3 Ks = vec3(0.8, 0.5, 0.5);
	
    vec3 normal = normalize(normalInterp);
    vec3 viewDir = normalize(-vertPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    float lambertian = max(dot(lightDir, normal), 0.0);
    float specAngle = max(dot(halfDir, normal), 0.0);
    float specular = pow(specAngle, Ns);
	
	float bias = 0.1;
    fragColor = vec4(Ka, 1.0)
              + texture(map_Shadow, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w)) * vec4(Kd, 1.0) * lambertian
              + texture(map_Shadow, vec3(shadowCoord.xy/shadowCoord.w, (shadowCoord.z - bias)/shadowCoord.w)) * vec4(Ks, 1.0) * specular;

}
