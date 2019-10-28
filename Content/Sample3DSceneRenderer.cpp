#include "pch.h"
#include "Sample3DSceneRenderer.h"
#include "..\Common\DirectXHelper.h"

using namespace LearnDirectx;

using namespace DirectX;
using namespace Windows::Foundation;

Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void Sample3DSceneRenderer::CreateWindowSizeDependentResources() 
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// Right handed axis
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	static const XMVECTORF32 eye = {0.0f, 0.7f, 1.5f, 0.0f};
	static const XMVECTORF32 target = {0.0f, -0.1f, 0.0f, 0.0f};
	static const XMVECTORF32 up = {0.0f, 1.0f, 0.0f, 0.0f};

	// https://stackoverflow.com/questions/32037617/why-is-this-transpose-required-in-my-worldviewproj-matrix
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(
		XMMatrixLookAtRH(eye, target, up)
	));

	m_constantBufferData.cameraPos = XMFLOAT4(0.0f, 0.7f, 1.5f, 0.0f);
}

void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking) {
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);
	}
}

void Sample3DSceneRenderer::Rotate(float radians) 
{
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::StartTracking() {
	m_tracking = true;
}

void Sample3DSceneRenderer::TrackingUpdate(float positionX) 
{
	if (m_tracking) {
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking() {
	m_tracking = false;
}

void Sample3DSceneRenderer::Render() 
{
	if (!m_loadingComplete) {
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
	);

	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT,
		0
	);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
	);

	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->PSSetConstantBuffers1(
		1,
		1,
		m_constantPixelBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	// Set textures and samplers
	context->PSSetSamplers(0, 1, m_myTexSamplerState.GetAddressOf());
	context->PSGetShaderResources(0, 1, m_myTexResView.GetAddressOf());

	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
	);

	context->DrawIndexed(
		m_indexCount,
		0,
		0
	);
}


void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				m_vertexShader.ReleaseAndGetAddressOf()
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				m_inputLayout.ReleaseAndGetAddressOf()
			)
		);
	});

	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData){
		m_deviceResources->GetD3DDevice()->CreatePixelShader(
			&fileData[0],
			fileData.size(),
			nullptr,
			m_pixelShader.ReleaseAndGetAddressOf()
		);

		// create constant buffer (MVP)
		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				m_constantBuffer.ReleaseAndGetAddressOf()
			)
		);

		// create constant buffer for pixel shader
		static const PixelConstantBuffer pixelParameters = { 
			XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
			XMFLOAT4(0.717f, 0.717f, 0, 0.0f),
			XMFLOAT4(0.25f, 0.25f, 0.5f, 1.0f),
			XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f),
			XMFLOAT4(20.0f, 0.0f, 0.0f, 0.0f),
		};
		D3D11_SUBRESOURCE_DATA pixelConstantBufferData = { 0 };
		pixelConstantBufferData.pSysMem = &pixelParameters;
		pixelConstantBufferData.SysMemPitch = 0;
		pixelConstantBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC constantPixelBufferDesc(sizeof(PixelConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantPixelBufferDesc,
				&pixelConstantBufferData,
				m_constantPixelBuffer.ReleaseAndGetAddressOf()
			)
		);
	});

	auto createCubeTask = (createPSTask && createVSTask).then([this]() {
		
		// create cube vertices buffer
		static const VertexPositionColor cubeVertices[] = 
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(-0.577f,-0.577f,-0.577f), XMFLOAT2(0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-0.577f,-0.577f,0.577f), XMFLOAT2(0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(-0.577f,0.577f,-0.577f), XMFLOAT2(1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT3(-0.577f,0.577f,0.577f), XMFLOAT2(1.0f, 1.0f)},
			{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.577f,-0.577f,0.577f), XMFLOAT2(1.0f, 0.0f)},
			{XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.577f,-0.577f,0.577f), XMFLOAT2(1.0f, 1.0f)},
			{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.577f,0.577f,-0.577f), XMFLOAT2(0.0f, 1.0f)},
			{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.577f,0.577f,0.577f), XMFLOAT2(0.0f, 0.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				m_vertexBuffer.ReleaseAndGetAddressOf()
			)
		);

		// create index buffer
		static const unsigned short cubeIndices[] =
		{
			0,2,1, // -x
			1,2,3,

			4,5,6, // +x
			5,7,6,

			0,1,5, // -y
			0,5,4,

			2,6,7, // +y
			2,7,3,

			0,4,6, // -z
			0,6,2,

			1,3,7, // +z
			1,7,5,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				m_indexBuffer.ReleaseAndGetAddressOf()
			)
		);
	});

	//createCubeTask.then([this]() {
	//	m_loadingComplete = true;
	//});

	auto loadTextureTask = createCubeTask.then([this]() {
		// initialize texture
		DX::ThrowIfFailed(
			DirectX::CreateDDSTextureFromFile(
				m_deviceResources->GetD3DDevice(),
				L"Assets\\Texture\\WoodCrate.dds",
				m_myTexRes.GetAddressOf(),
				m_myTexResView.GetAddressOf())
		);

		// initialize sampler
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateSamplerState(
				&sampDesc,
				m_myTexSamplerState.GetAddressOf()
			)
		);
	});

	loadTextureTask.then([this]() {
		m_loadingComplete = true;
	});

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources() 
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_constantPixelBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}