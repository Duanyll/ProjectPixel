#version 330 core

struct ParticleMaterial {
    sampler2D diffuse;
    sampler2D emission;
    vec4 color;
}; 

uniform ParticleMaterial material;
vec3 diffuseCol;
vec3 viewDir, normal;

struct DirLight {
    vec3 direction;

    vec3 diffuse;
    vec3 specular;

    mat4 lightSpace;
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

    mat4 lightSpace;
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
uniform sampler2D dirLightDepth;
uniform sampler2D spotLightDepth;

uniform Camera {
    mat4 projection;
    mat4 view;   
    vec3 viewPos; 
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 FragPosDirLight;
    vec3 FragPosSpotLight;
};

out vec4 FragColor;

float CalcDirLightShadow() {
    float currentDepth = FragPosDirLight.z;
    if (currentDepth >= 1.0) return 1.0;
    float shadow = 0.0;
    float eps = 0.003;
    vec2 texelSize = 1.0 / textureSize(dirLightDepth, 0);
    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(dirLightDepth, FragPosDirLight.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - eps > pcfDepth ? 1.0 : 0.0;        
        }    
    }   
    shadow /= 9.0;
    return 1 - shadow;
}

vec3 CalcDirLight(DirLight light) {
    float shadow = CalcDirLightShadow();
    if (shadow <= 0.0) return vec3(0.0, 0.0, 0.0); 
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 合并结果
    vec3 diffuse  = light.diffuse  * diff * diffuseCol;
    return diffuse * shadow;
}

vec3 CalcPointLight(PointLight light) {
    vec3 lightDir = normalize(light.position - FragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 衰减
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    // 合并结果
    vec3 diffuse  = light.diffuse  * diff * diffuseCol;
    diffuse  *= attenuation;
    return diffuse;
}

float CalcSpotLightShadow() {
    float eps = 0.005;
    float currentDepth = FragPosSpotLight.z;
    if (currentDepth >= 1.0) return 0.0;
    float depth = texture(spotLightDepth, FragPosSpotLight.xy).r;
    return currentDepth - eps > depth ? 0.0 : 1.0;
}

vec3 CalcSpotLight(SpotLight light) {
    float shadow = CalcSpotLightShadow();
    if (shadow <= 0.0) return vec3(0.0, 0.0, 0.0); 
    vec3 lightDir = normalize(light.position - FragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 衰减
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);    
    // 合并结果
    vec3 diffuse  = light.diffuse  * diff * diffuseCol;
    diffuse  *= attenuation * intensity;
    return diffuse * shadow;
}

void main() {
    normal = normalize(Normal);
    viewDir = normalize(viewPos - FragPos);

    diffuseCol = texture(material.diffuse, TexCoord).rgb;
    vec4 emission = texture(material.emission, TexCoord);
    vec3 result = vec3(emission) * emission.a;

    result += ambientLight  * vec3(diffuseCol);

    if (useDirLight)
        result += CalcDirLight(dirLight);
    for (int i = 0; i < pointLightCount; i++)
        result += CalcPointLight(pointLights[i]);    
    if (useSpotLight)
        result += CalcSpotLight(spotLight);

    FragColor = vec4(result, 1.0) * material.color;
}