#pragma once

namespace LearnDirectx
{
	// 用于向顶点着色器发送 MVP 矩阵的常量缓冲区。
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4 cameraPos;
	};

	// 用于向顶点着色器发送每个顶点的数据。
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
	};

	struct PixelConstantBuffer
	{
		DirectX::XMFLOAT4 mainColor;
		DirectX::XMFLOAT4 lightPos;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;
		DirectX::XMFLOAT4 specular_falloff;
	};
}