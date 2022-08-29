
struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 pixelPosition : PIXEL_POSITION; 
};

cbuffer SystemCBuf : register(b0)
{
    float4x4 viewProjection;
};

cbuffer UserCBuf : register(b1)
{
    float4x4 transform;
    float4x4 normalMatrix;
};


VSOutput main(VSInput input)
{
    VSOutput vso;
    vso.position = mul(float4(input.position, 1.0f), mul(transform, viewProjection));
    vso.texCoord = input.texCoord;
    vso.normal = mul(input.normal, (float3x3)normalMatrix);
    vso.pixelPosition = (float3)mul(float4(input.position, 1.0f), transform);
    return vso;
}

