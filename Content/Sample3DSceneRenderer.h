#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

namespace LearnDirectx
{
	class Sample3DSceneRenderer 
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }
			
	private:
		void Rotate(float radians);

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantPixelBuffer;
		Microsoft::WRL::ComPtr<ID3D11Resource> m_myTexRes;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_myTexResView;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_myTexSamplerState;

		ModelViewProjectionConstantBuffer m_constantBufferData;
		uint32 m_indexCount;

		bool m_loadingComplete;
		float m_degreesPerSecond;
		bool m_tracking;
	};
}