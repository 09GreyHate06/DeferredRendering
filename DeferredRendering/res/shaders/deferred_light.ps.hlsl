
struct DirectionalLight
{
    float3 direction;
    float p0;
    float3 ambient;
    float p1;
    float3 diffuse;
    float p2;
    float3 specular;
    float p3;
    
    // for shadow calculation
    //float4x4 viewProjMatrix;
};

struct PointLight
{
    float3 position;
    float attConstant;
    float3 ambient;
    float attLinear;
    float3 diffuse;
    float attQuadratic;
    float3 specular;
    float p0;
    
    // for shadow calculation
    //float farPlane;
    //float nearPlane;
    //float p1;
    //float p2;
    //float4x4 viewMatrix;
};

struct SpotLight
{
    float3 direction;
    float attConstant;
    float3 position;
    float attLinear;
    
    float3 ambient;
    float attQuadratic;
    float3 diffuse;
    float innerCutOffAngleCos;
    float3 specular;
    float outerCutOffAngleCos;
    
    // for shadow calculation
    //float4x4 viewProjMatrix;
};

struct PhongInput
{
    float3 pixelToLight;
    float3 pixelToView;
    float3 normal;
    
    struct
    {
        float3 diffuse;
        float3 specular;
        float shininess;
    } mat;
    
    struct
    {
        float3 ambient;
        float3 diffuse;
        float3 specular;
    } light;
};

static const uint s_lightMaxCount = 32;

cbuffer SystemCBuf : register(b0)
{
    DirectionalLight dirLights[s_lightMaxCount];
    PointLight pointLights[s_lightMaxCount];
    SpotLight spotLights[s_lightMaxCount];
    
    float3 viewPosition;
    float p0;
    
    uint activeDirLights = 0;
    uint activePointLights = 0;
    uint activeSpotLights = 0;
    float p1;
}

Texture2D gPosition   : register(t0);
Texture2D gNormal     : register(t1);
Texture2D gDiffuse    : register(t2);
Texture2D gSpecular   : register(t3);
Texture2D gShininess  : register(t4);
SamplerState gSampler : register(s0);

float3 Phong(PhongInput input);
float CalcAttenuation(float distance, float attConstant, float attLinear, float attQuadratic);



float4 main(float2 texCoord : TEXCOORD) : SV_Target
{
    float3 pixelPosition = gPosition.Sample(gSampler, texCoord).xyz;
    float3 normal        = normalize(gNormal.Sample(gSampler, texCoord).xyz);
    float3 diffuse      = gDiffuse.Sample(gSampler, texCoord).rgb; // deferred doesn't support alpha blending
    float3 specular     = gSpecular.Sample(gSampler, texCoord).rgb; // deferred doesn't support alpha blending
    float shininess      = gShininess.Sample(gSampler, texCoord).r;
    
    
    
    float3 dirLightPhong = float3(0.0f, 0.0f, 0.0f);
    float3 pointLightPhong = float3(0.0f, 0.0f, 0.0f);
    float3 spotLightPhong = float3(0.0f, 0.0f, 0.0f);
    
    float3 pixelToView = normalize(viewPosition - pixelPosition);
    for (int i = 0; i < activeDirLights; i++)
    {
        DirectionalLight light = dirLights[i];
        
        PhongInput phongInput;
        phongInput.pixelToLight = normalize(-light.direction);
        phongInput.pixelToView = pixelToView;
        phongInput.normal = normal;
        phongInput.light.ambient = light.ambient;
        phongInput.light.diffuse = light.diffuse;
        phongInput.light.specular = light.specular;
        phongInput.mat.diffuse = diffuse;
        phongInput.mat.specular = specular;
        phongInput.mat.shininess = shininess;
        
        dirLightPhong += Phong(phongInput);
    }

    for (int i = 0; i < activePointLights; i++)
    {
        PointLight light = pointLights[i];
        
        PhongInput phongInput;
        phongInput.pixelToLight = normalize(light.position - pixelPosition);
        phongInput.pixelToView = pixelToView;
        phongInput.normal = normal;
        phongInput.light.ambient = light.ambient;
        phongInput.light.diffuse = light.diffuse;
        phongInput.light.specular = light.specular;
        phongInput.mat.diffuse = diffuse;
        phongInput.mat.specular = specular;
        phongInput.mat.shininess = shininess;
        
        float attenuation = CalcAttenuation(length(light.position - pixelPosition), light.attConstant, light.attLinear, light.attQuadratic);
        
        pointLightPhong += Phong(phongInput) * attenuation;
    }
    
    for (int i = 0; i < activeSpotLights; i++)
    {
        SpotLight light = spotLights[i];
        
        PhongInput phongInput;
        phongInput.pixelToLight = normalize(light.position - pixelPosition);
        phongInput.pixelToView = pixelToView;
        phongInput.normal = normal;
        phongInput.light.ambient = light.ambient;
        phongInput.light.diffuse = light.diffuse;
        phongInput.light.specular = light.specular;
        phongInput.mat.diffuse = diffuse;
        phongInput.mat.specular = specular;
        phongInput.mat.shininess = shininess;
        
        float attenuation = CalcAttenuation(length(light.position - pixelPosition), light.attConstant, light.attLinear, light.attQuadratic);
        
        float theta = dot(phongInput.pixelToLight, normalize(-light.direction));
        float epsilon = light.innerCutOffAngleCos - light.outerCutOffAngleCos;
        float intensity = clamp((theta - light.outerCutOffAngleCos) / epsilon, 0.0f, 1.0f);
        
        spotLightPhong += Phong(phongInput) * attenuation * intensity;
    }
    
    return float4(dirLightPhong + pointLightPhong + spotLightPhong, 1.0f);
}

float3 Phong(PhongInput input)
{
    float3 ambient = input.light.ambient * input.mat.diffuse;
    
    float diffuseFactor = max(dot(input.normal, input.pixelToLight), 0.0f);
    float3 diffuse = input.light.diffuse * input.mat.diffuse * diffuseFactor;
    
    float3 halfwayDir = normalize(input.pixelToLight + input.pixelToView);
    float specularFactor = pow(max(dot(input.normal, halfwayDir), 0.0f), input.mat.shininess);
    float3 specular = input.light.specular * input.mat.specular * specularFactor;
    
    return ambient + diffuse + specular;
}

float CalcAttenuation(float distance, float attConstant, float attLinear, float attQuadratic)
{
    return 1.0f / (attConstant + attLinear * distance + attQuadratic * (distance * distance));
}