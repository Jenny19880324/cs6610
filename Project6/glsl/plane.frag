#version 330
in vec3 normalInterp;
in vec3 vertPos;
//in vec2 texCoord;

//uniform sampler2D map_Kd;

out vec4 fragColor;

void main(void){
    vec3 normal = normalize(normalInterp);
    vec3 viewDir = normalize(-vertPos);
    vec3 reflectDir = -viewDir + 2 * dot(viewDir, normal) * normal;

    //fragColor = texture(map_Kd, texCoord);
    fragColor = vec4(1.0, 0, 0, 1);
}

