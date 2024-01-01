#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
uniform int NR_LIGHTS;
uniform Light lights[100];
uniform bool EnableHDR;
uniform float Exposure;

void main()
{            
    const float gamma = 2.2; 
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    
    // then calculate lighting as usual
    vec3 ambient = vec3(0.05 * Diffuse * AmbientOcclusion);
    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        float distance = length(lights[i].Position - FragPos);
        if (distance < lights[i].Radius)
        {
            // diffuse
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
            vec3 specular = lights[i].Color * spec;
            // attenuation
            //float distance = length(light.Position - FragPos);
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }

    //vec3 color = vec3(0.03) * ambient;
    vec3 color = ambient * lighting;

    //// HDR tonemapping
    //color = color / (color + vec3(1.0));
    //// gamma correct
    if (EnableHDR)
    {
        vec3 result = vec3(1.0) - exp(-color * Exposure);
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        vec3 result = pow(color, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}