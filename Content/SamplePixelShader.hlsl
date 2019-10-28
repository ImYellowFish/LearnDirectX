Texture2D myTex : register(t0);
SamplerState mySampler : register(s0);

cbuffer ModelViewProjectionConstantBuffer : register(b1)
{
    float4 mainColor;
    float4 lightPos;
    float4 diffuse;
    float4 specular;
    float4 specular_falloff;
};

// 通过像素着色器传递的每个像素的颜色数据。
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
    float3 normal : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float2 uv : TEXCOORD2;
};

// (内插)颜色数据的传递函数。
float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 halfVec = normalize(lightPos.xyz + input.viewDir);

    float3 diff = diffuse * max(dot(normal, lightPos.xyz), 0.0f);

    float3 spec = specular * pow(max(dot(normal, halfVec), 0.0f), specular_falloff.x);
    float3 color = myTex.Sample(mySampler, input.uv).xyz;
    return float4(color, 1.0f);
}
