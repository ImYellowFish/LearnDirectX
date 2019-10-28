Texture2D myTex : register(t0);
SamplerState mySampler : register(s0);

// 存储用于构成几何图形的三个基本列优先矩阵的常量缓冲区。
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
    float4 cameraPos;
};

// 用作顶点着色器输入的每个顶点的数据。
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
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

// 用于在 GPU 上执行顶点处理的简单着色器。
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// 将顶点位置转换为投影空间。
	float4 worldPos = mul(pos, model);
    pos = mul(worldPos, view);
	pos = mul(pos, projection);
	output.pos = pos;
    
    output.normal = mul(float4(input.normal, 0.0f), model);
    output.viewDir = cameraPos.xyz - worldPos.xyz;

	// 不加修改地传递颜色。
	output.color = input.color;
    output.uv = input.uv;

	return output;
}
