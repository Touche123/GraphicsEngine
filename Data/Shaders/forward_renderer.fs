#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 directionalLightDirection;
uniform vec3 directionalLightColor;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform vec3 viewPos;
uniform vec3 ambient;

#define NR_POINT_LIGHTS 1

uniform PointLight pointLights[NR_POINT_LIGHTS];

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    //vec4 shadowMapPos = vec4(fs_in.FragPos, 1.0);
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    //vec3 lightDir = lightPos;
    //vec3 lightDir = normalize(-directionalLightDirection);
    vec3 lightDir = normalize(directionalLightDirection - fs_in.FragPos);
    float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.0001);

    // PCF
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    // Shadow Factor
    float shadowFactor = 0.0;

    const int numSamples = 32; // Increase this value for more samples

    for(int i = 0; i < numSamples; ++i)
    {
        vec2 offset = vec2(
            cos(float(i) * 6.283185 / float(numSamples)),
            sin(float(i) * 6.283185 / float(numSamples))
        ) * texelSize;

        float pcfDepth = texture(shadowMap, projCoords.xy + offset).r; 
        shadowFactor += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    }

    shadowFactor /= float(numSamples);

    //keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadowFactor = 0.0;
        
    return shadowFactor;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.5);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(diffuseTexture, fs_in.TexCoords));

    return ambient;
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    
    //vec3 directionalLightDir = normalize(-directionalLightDirection);
    vec3 directionalLightDir = normalize(directionalLightDirection - fs_in.FragPos);
    float directionalLightdiff = max(dot(directionalLightDir, normal), 0.0);

    vec3 pointLightDir = normalize(pointLightPos - fs_in.FragPos);
    float pointLightDiff = max(dot(pointLightDir, normal), 0.0);

    vec3 diffuse = (directionalLightdiff * directionalLightColor) + (pointLightDiff * pointLightColor);
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-directionalLightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(directionalLightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * directionalLightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(pow(lighting, vec3(1.0/2.2)), 1.0);
}