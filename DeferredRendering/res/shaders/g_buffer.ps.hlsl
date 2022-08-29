struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 pixelPosition : PIXEL_POSITION;
};

struct PSOutput
{
    float4 gPosition : SV_Target0;
    float4 gNormal : SV_Target1;
    float4 gDiffuse : SV_Target2;
    float4 gSpecular : SV_Target3;
    float gShininess : SV_Target4;
};

struct Material
{
    float4 diffuseCol;
    float2 tiling;
    float shininess;
    float p0;
};

cbuffer UserCBuf : register(b0)
{
    Material material;
};


Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
SamplerState diffuseMapSampler : register(s0);
SamplerState specularMapSampler : register(s1);

PSOutput main(VSOutput input)
{
    PSOutput pso;
    pso.gPosition = float4(input.pixelPosition, 0.0f);
    pso.gNormal = float4(input.normal, 0.0f);
    pso.gDiffuse = diffuseMap.Sample(diffuseMapSampler, input.texCoord * material.tiling) * material.diffuseCol;
    pso.gSpecular = specularMap.Sample(specularMapSampler, input.texCoord * material.tiling);
    pso.gShininess = material.shininess;
    
    return pso;
}

