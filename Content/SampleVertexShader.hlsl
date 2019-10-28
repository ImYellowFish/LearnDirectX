Texture2D myTex : register(t0);
SamplerState mySampler : register(s0);

// �洢���ڹ��ɼ���ͼ�ε��������������Ⱦ���ĳ�����������
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
    float4 cameraPos;
};

// ����������ɫ�������ÿ����������ݡ�
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// ͨ��������ɫ�����ݵ�ÿ�����ص���ɫ���ݡ�
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
    float3 normal : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float2 uv : TEXCOORD2;
};

// ������ GPU ��ִ�ж��㴦��ļ���ɫ����
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// ������λ��ת��ΪͶӰ�ռ䡣
	float4 worldPos = mul(pos, model);
    pos = mul(worldPos, view);
	pos = mul(pos, projection);
	output.pos = pos;
    
    output.normal = mul(float4(input.normal, 0.0f), model);
    output.viewDir = cameraPos.xyz - worldPos.xyz;

	// �����޸ĵش�����ɫ��
	output.color = input.color;
    output.uv = input.uv;

	return output;
}
