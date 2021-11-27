#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
}; 

uniform Material material;
vec3 diffuseCol, specularCol;
vec3 viewDir, normal;

struct DirLight {
    vec3 direction;

    vec3 diffuse;
    vec3 specular;
};  

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4
uniform Lights {
    bool useDirLight;
    DirLight dirLight;
    int pointLightCount;
    PointLight pointLights[NR_POINT_LIGHTS];
    bool useSpotLight;
    SpotLight spotLight;
    vec3 ambientLight;
};

uniform Camera {
    mat4 projection;
    mat4 view;   
    vec3 viewPos; 
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
};

out vec4 FragColor;

vec3 CalcDirLight(DirLight light) {
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // 合并结果
    vec3 diffuse  = light.diffuse  * diff * diffuseCol;
    vec3 specular = light.specular * spec * specularCol;
    return (diffuse + specular);
}

vec3 CalcPointLight(PointLight light) {
    vec3 lightDir = normalize(light.position - FragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // 衰减
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    // 合并结果
    vec3 diffuse  = light.diffuse  * diff * diffuseCol;
    vec3 specular = light.specular * spec * specularCol;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light) {
    vec3 lightDir = normalize(light.position - FragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // 衰减
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    
    // 合并结果
    vec3 diffuse  = light.diffuse  * diff * diffuseCol;
    vec3 specular = light.specular * spec * specularCol;
    diffuse  *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (diffuse + specular);
}

void main() {
    if (texture(material.diffuse, TexCoord).a < 0.5) discard;
    normal = normalize(Normal);
    viewDir = normalize(viewPos - FragPos);

    diffuseCol = texture(material.diffuse, TexCoord).rgb;
    vec4 specular = texture(material.specular, TexCoord);
    specularCol = vec3(specular) * specular.a;
    vec4 emission = texture(material.emission, TexCoord);
    vec3 result = vec3(emission) * emission.a;

    result += ambientLight  * vec3(diffuseCol);

    if (useDirLight)
        result += CalcDirLight(dirLight);
    for (int i = 0; i < pointLightCount; i++)
        result += CalcPointLight(pointLights[i]);    
    if (useSpotLight)
        result += CalcSpotLight(spotLight);    

    FragColor = vec4(result, 1.0);
}