#version 330
in vec3 normalInterp;
in vec3 vertPos;

uniform float Ns; //Specular exponent
uniform vec3 Ka;  //Ambient color;
uniform vec3 Kd;  //Diffuse color
uniform vec3 Ks;  //Specular color
uniform samplerCube cubemap;

layout(location = 0) out vec4 fragColor;

uniform vec3 lightPosition;

void main(void){
    vec3 normal = normalize(normalInterp);
    vec3 lightDir = normalize(lightPosition);
    lightDir = normalize(vec3(0, 1, 0));
    vec3 viewDir = normalize(-vertPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    vec3 reflectDir = -viewDir + 2 * dot(viewDir, normal) * normal;

    float lambertian = max(dot(lightDir, normal), 0.0);
    float specAngle = max(dot(halfDir, normal), 0.0);
    float specular = pow(specAngle, Ns);
    vec3 textureDir = reflectDir;
    fragColor = vec4(Ka, 1.0) * texture(cubemap, textureDir)
              + vec4(Kd, 1.0) * texture(cubemap, textureDir) * lambertian
              + vec4(Ks, 1.0) * specular;

}
