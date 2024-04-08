#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 ambient;

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
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
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

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(pow(lighting, vec3(1.0/2.2)), 1.0);
}