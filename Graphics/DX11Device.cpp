#include "pch.h"
#include "DX11Device.h"

#include <DirectXTex\DirectXTex.h>
#include "..\Libraries\DXTK\Inc\WICTextureLoader.h"
#include "..\Libraries\DXTK\Inc\DDSTextureLoader.h"

#include <d3dcompiler.h>
#include <filesystem>

#pragma comment (lib,"d3dcompiler.lib")
#pragma comment (lib,"DirectXTex.lib")


#define CompileCSO




DX11Device::DX11Device()
	: m_pDevice(nullptr)
	, m_pDeviceContext(nullptr)
	, m_FeatureLevel()
	, m_pScreenDepthView(nullptr)
	, m_pScreenDepthBuffer(nullptr)
	, m_ScreenViewport()
	, m_hWnd(0)
	, m_Enable4xMsaa(false)
	, m_4xMsaaQuality(0)
{
	m_D3DDriverType = D3D_DRIVER_TYPE_HARDWARE;
}

DX11Device::~DX11Device()
{
	// Restore all default settings.
	ReleaseCOM(m_pDeviceContext);
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
	}
}


HRESULT DX11Device::Create(HWND hWnd, int screenWidth, int screenHeight, float _GameScreenSize)
{

	m_ScreenWidth = screenWidth;
	m_ScreenHeight = screenHeight;
	m_pConstantBuffer.resize((int)CB_Type::END);

	// 디바이스 생성 모드. 디버그 빌드인 경우 디버그 모드로
	UINT _createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	/*_createDeviceFlags = _createDeviceFlags| D3D11_CREATE_DEVICE_DEBUG;*/

#endif
	//_createDeviceFlags = _createDeviceFlags | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	// 드라이버 타입
	m_D3DDriverType = D3D_DRIVER_TYPE_HARDWARE;

	// 스왑체인 정보
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = screenWidth;
	sd.BufferDesc.Height = screenHeight;
	sd.BufferDesc.RefreshRate.Numerator = 120;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = (HWND)hWnd;		// DXGI-Windef
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;
	sd.SampleDesc.Count = 1;			// No MSAA
	sd.SampleDesc.Quality = 0;


	
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		0, m_D3DDriverType, 0, _createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &sd,
		m_pDrawSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), &m_FeatureLevel, &m_pDeviceContext);

	sd.BufferDesc.Width = screenWidth * _GameScreenSize;
	sd.BufferDesc.Height = screenHeight * _GameScreenSize;


	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return hr;
	}

	if (m_FeatureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return hr;
	}


	ID3D11Texture2D* buffer;
	HR(m_pDrawSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&buffer)));
	if (buffer != nullptr)
		HR(m_pDevice->CreateRenderTargetView(buffer, 0, m_RenderTargetView.GetAddressOf()));
	ReleaseCOM(buffer);
#pragma region RenderTarget
	//==============================================================================================
	//screen



	D3D11_TEXTURE2D_DESC targetDesc{};
	targetDesc.Width = screenWidth * _GameScreenSize;
	targetDesc.Height = screenHeight * _GameScreenSize;
	targetDesc.MipLevels = 1;
	targetDesc.ArraySize = 1;
	targetDesc.SampleDesc.Count = 1;		// No MSAA
	targetDesc.SampleDesc.Quality = 0;
	targetDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	targetDesc.Usage = D3D11_USAGE_DEFAULT;
	targetDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	targetDesc.CPUAccessFlags = 0;
	targetDesc.MiscFlags = 0;

	hr = m_pDevice->CreateTexture2D(&targetDesc,nullptr, m_pScreenTargetBuffer.GetAddressOf());

	if (m_pScreenTargetBuffer.Get() != nullptr)
		HR(m_pDevice->CreateRenderTargetView(m_pScreenTargetBuffer.Get(), 0, m_pScreenTargetView.GetAddressOf()));

	// Create the depth/stencil buffer and view.
	/// 뎁스-스텐실 버퍼와 뷰를 생성한다.

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = screenWidth * _GameScreenSize;
	depthStencilDesc.Height = screenHeight * _GameScreenSize;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;		// No MSAA
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pScreenDepthBuffer));
	if (m_pScreenDepthBuffer != nullptr)
		HR(m_pDevice->CreateDepthStencilView(m_pScreenDepthBuffer.Get(), 0, m_pScreenDepthView.GetAddressOf()));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	m_pDevice->CreateShaderResourceView(m_pScreenTargetBuffer.Get(),&srvDesc,m_pScreenSRV.GetAddressOf() );

	// Set the viewport transform.
	/// 뷰포트 변환을 셋팅한다.
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(screenWidth * _GameScreenSize);
	m_ScreenViewport.Height = static_cast<float>(screenHeight * _GameScreenSize);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;
	//==============================================================================================
	// Shadow
	{
		D3D11_TEXTURE2D_DESC ShadowDesc;
		ShadowDesc = depthStencilDesc;
		ShadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		ShadowDesc.Width = screenWidth * _GameScreenSize;
		ShadowDesc.Height = screenHeight * _GameScreenSize;
		ShadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		HR(m_pDevice->CreateTexture2D(&ShadowDesc, 0, m_pShdowDepthBuffer.GetAddressOf()));
		if (m_pShdowDepthBuffer != nullptr)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
			descDSV.Format = DXGI_FORMAT_D32_FLOAT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

			HR(m_pDevice->CreateDepthStencilView(m_pShdowDepthBuffer.Get(), &descDSV, m_pShadowDepthView.GetAddressOf()));
		}
		ShadowDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
		ShadowDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		HR(m_pDevice->CreateTexture2D(&ShadowDesc, 0, &buffer));
		HR(m_pDevice->CreateRenderTargetView(buffer, 0, m_pShadowTargetView.GetAddressOf()));
		ReleaseCOM(buffer);

		D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
		shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shadowSRVDesc.Texture2D.MipLevels = 1;

		m_pDevice->CreateShaderResourceView(m_pShdowDepthBuffer.Get(), &shadowSRVDesc, m_pShdowSRV.GetAddressOf());
		m_ShadowViewport = m_ScreenViewport;

		m_ShadowViewport.Width = screenWidth * _GameScreenSize;
		m_ShadowViewport.Height = screenHeight * _GameScreenSize;

	}


	//==============================================================================================
	//UI

	targetDesc.Width = screenWidth;
	targetDesc.Height = screenHeight;

	m_pDevice->CreateTexture2D(&targetDesc, 0, m_pUITargetBuffer.GetAddressOf());

	if (m_pUITargetBuffer.Get() != nullptr)
		HR(m_pDevice->CreateRenderTargetView(m_pUITargetBuffer.Get(), 0, m_pUITargetView.GetAddressOf()));

	depthStencilDesc.Width = screenWidth;
	depthStencilDesc.Height = screenHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;		// No MSAA
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pDevice->CreateTexture2D(&depthStencilDesc, 0, m_pUIDepthBuffer.GetAddressOf()));
	if (m_pUIDepthBuffer.Get() != nullptr)
		HR(m_pDevice->CreateDepthStencilView(m_pUIDepthBuffer.Get(), 0, m_pUIDepthView.GetAddressOf()));

	m_pDevice->CreateShaderResourceView(m_pUITargetBuffer.Get(), &srvDesc, m_pUISRV.GetAddressOf());


	m_UIViewport = m_ScreenViewport;
	m_UIViewport.Width = static_cast<float>(screenWidth);
	m_UIViewport.Height = static_cast<float>(screenHeight);

	m_Viewport = m_UIViewport;







	m_Vertex.resize(4);
	m_Indices.resize(6);

	m_Vertex[0].m_Pos = { -1,-1,0 };
	m_Vertex[1].m_Pos = { 1,-1,0 };
	m_Vertex[2].m_Pos = { -1,1,0 };
	m_Vertex[3].m_Pos = { 1,1,0 };

	m_Vertex[0].m_UVW = { 0,1,0 };
	m_Vertex[1].m_UVW = { 1,1,0 };
	m_Vertex[2].m_UVW = { 0,0,0 };
	m_Vertex[3].m_UVW = { 1,0,0 };

	m_Indices[0] = 0;
	m_Indices[1] = 2;
	m_Indices[2] = 1;

	m_Indices[3] = 2;
	m_Indices[4] = 3;
	m_Indices[5] = 1;

	CreateVertexBuffer(sizeof(Device::Vertex) * m_Vertex.size(),m_Vertex.data(),&m_pCombineVertexBuffer);
	CreateIndexBuffer(m_Indices.size(), m_Indices.data(), &m_pCombineIndexBuffer);
#pragma endregion



	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = true;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true; // 블렌딩 활성화
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 소스 색상의 알파 값을 사용
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 대상 색상의 알파 값을 사용
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // 덧셈 연산
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // 소스 알파 값에 1을 사용
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE; // 대상 알파 값을 1으로 사용
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // 덧셈 연산
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 모든 색상 채널에 대한 쓰기 활성화


	m_pDevice->CreateBlendState(&blendDesc, &m_pDefaultBlendState);
	m_pBlendState = m_pDefaultBlendState.Get();

#pragma region CreateRenderStates
	{
		D3D11_RASTERIZER_DESC solidDesc;
		ZeroMemory(&solidDesc, sizeof(D3D11_RASTERIZER_DESC));
		solidDesc.FillMode = D3D11_FILL_SOLID;
		solidDesc.CullMode = D3D11_CULL_NONE;
		solidDesc.FrontCounterClockwise = false;
		solidDesc.DepthClipEnable = true;

		HR(m_pDevice->CreateRasterizerState(&solidDesc, m_pDefaultRS.GetAddressOf()));

		D3D11_RASTERIZER_DESC spriteDesc{};
		ZeroMemory(&spriteDesc, sizeof(D3D11_RASTERIZER_DESC));
		spriteDesc.FillMode = D3D11_FILL_SOLID;
		spriteDesc.CullMode = D3D11_CULL_BACK;
		spriteDesc.FrontCounterClockwise = false;
		spriteDesc.DepthClipEnable = false;

		HR(m_pDevice->CreateRasterizerState(&spriteDesc, m_pSpriteRasterizerState.GetAddressOf()));

		D3D11_DEPTH_STENCIL_DESC equalsDesc;
		ZeroMemory(&equalsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		equalsDesc.DepthEnable = true;
		equalsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;		// 깊이버퍼에 쓰기는 한다
		equalsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		
		HR(m_pDevice->CreateDepthStencilState(&equalsDesc, m_pDefaultDepthState.GetAddressOf()));

		D3D11_DEPTH_STENCIL_DESC CombineDS_Desc;
		ZeroMemory(&CombineDS_Desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		CombineDS_Desc.DepthEnable = false;
		CombineDS_Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;		// 깊이버퍼에 쓰기는 한다
		CombineDS_Desc.DepthFunc = D3D11_COMPARISON_NEVER;					// 언제나 쓴다.

		HR(m_pDevice->CreateDepthStencilState(&equalsDesc, m_pCombineDepthState.GetAddressOf()));

	}
#pragma endregion

#pragma region CreateShader
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}

		};
		// staticMesh VS
		CreateVS(L"StaticVS", &m_pStaticVS.VS, layout, &m_pStaticVS.Layout);
		CreateVS(L"SpriteVS", &m_pSpriteVS.VS, layout, &m_pSpriteVS.Layout);
	}
	{
		// SkeletalMesh VS
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{ "WEIGHTS",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "BONEINDICES",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }

		};
		CreateVS(L"SkeletalVS", &m_pSkeletalVS.VS, layout, &m_pSkeletalVS.Layout);
	}
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		};
		CreateVS(L"CombineVS", &m_pCombine.VS, layout, &m_pCombine.Layout);
	}
	// PS
	CreatePS(L"DefaultPS", &m_pDefaultPS);
	CreatePS(L"SpritePS",&m_pSpritePS );
	CreatePS(L"CombinePS", &m_pCombinePS);




	SetVertexShader(m_pStaticVS);
	SetPixelShader(m_pDefaultPS);

	{
		D3D11_SAMPLER_DESC desc{};
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;				// 비등방 필터X
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		m_pDevice->CreateSamplerState(&desc, m_pLinearSampler.GetAddressOf());

		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_pDevice->CreateSamplerState(&desc, m_pClampSampler.GetAddressOf());

	}

	m_pDeviceContext->PSSetSamplers(0, 1, m_pLinearSampler.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(1, 1, m_pClampSampler.GetAddressOf());
#pragma endregion

	


	return S_OK;
}

void DX11Device::OnResize(int screenWidth, int screenHeight)
{
	// 이것들은 꼭 있어야 한다.
	assert(m_pDeviceContext);
	assert(m_pDevice);
	assert(m_pDrawSwapChain.Get());

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	/// 없애야 할 버퍼에 대한 참조를 가지고 있으므로 예전 뷰를 릴리즈한다.
	/// 또한 예전 뎁스-스텐실 버퍼도 릴리즈한다.


	// Resize the swap chain and recreate the render target view.
	/// 스왑체인의 크기를 변경하고 렌더타겟 뷰를 다시 생성한다.

	HR(m_pDrawSwapChain->ResizeBuffers(1, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(m_pDrawSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	if (backBuffer != nullptr)
		HR(m_pDevice->CreateRenderTargetView(backBuffer, 0, m_RenderTargetView.GetAddressOf()));
	ReleaseCOM(backBuffer);

	// Create the depth/stencil buffer and view.
	/// 뎁스-스텐실 버퍼와 뷰를 생성한다.

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = screenWidth;
	depthStencilDesc.Height = screenHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;		// No MSAA
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pScreenDepthBuffer));
	if (m_pScreenDepthBuffer != nullptr)
		HR(m_pDevice->CreateDepthStencilView(m_pScreenDepthBuffer.Get(), 0, m_pScreenDepthView.GetAddressOf()));


	// Bind the render target view and depth/stencil view to the pipeline.
	/// 렌더타겟뷰, 뎁스/스탠실뷰를 파이프라인에 바인딩한다.
	m_pDeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_pScreenDepthView.Get());


	// Set the viewport transform.
	/// 뷰포트 변환을 셋팅한다.
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(screenWidth);
	m_ScreenViewport.Height = static_cast<float>(screenHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;

	m_UIViewport.Width = static_cast<float>(screenWidth);
	m_UIViewport.Height = static_cast<float>(screenHeight);


}

void DX11Device::OnBeginRender()
{
	assert(m_pDeviceContext);


	// 마찬가지로 렌더 타겟도 다시 셋팅
	m_pDeviceContext->OMSetRenderTargets(1, m_pScreenTargetView.GetAddressOf(), m_pScreenDepthView.Get());


	// 깊이버퍼 그림자용으로 광원 기준으로 찍을 때 뷰포트를 건드리기 때문에, 다시 복구
	m_pDeviceContext->RSSetViewports(1, &m_ScreenViewport);

	// 여기서 각종 쉐이더 및 스테이트 디폴트값으로 초기화

	// 화면을 지운다.
	static float color[] = {0.0f,0.0f,0.0f,0};
	m_pDeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), color);
	m_pDeviceContext->ClearRenderTargetView(m_pScreenTargetView.Get(), color);
	m_pDeviceContext->ClearRenderTargetView(m_pUITargetView.Get(), color);



	ClearDepthView();

	m_RasterizerState = m_pDefaultRS.Get();
	m_pDepthState = m_pDefaultDepthState.Get();
	SetVertexShader(m_pStaticVS);
	SetPixelShader(m_pDefaultPS);
}

void DX11Device::RenderPipeline()
{
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 화면에 그릴 도구들을 준비한다.

	// IA
	m_pDeviceContext->IASetVertexBuffers(0,1,&m_pVertexBuffer,&m_Stride,&m_Offset);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);


	// VS
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	// 테셀레이션 단계 생략.

	// RS
	m_pDeviceContext->RSSetState(m_RasterizerState);
	// PS

	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

	// OM
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthState, 0);
	m_pDeviceContext->OMSetBlendState(m_pBlendState, NULL, 0xffffffff);
}

void DX11Device::Draw()
{
	// 화면을 그린다.
	m_pDeviceContext->DrawIndexed(m_IndexCount,0,0);
}


void DX11Device::OnEndRender()
{
	//m_pScreenSwapChain->Present(1, 0);
	//m_pUISwapChain->Present(1, 0);

	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), nullptr);
	m_pPixelShader = m_pSpritePS.Get();

	SetVertexBuffer(m_pCombineVertexBuffer,sizeof(Device::Vertex),0);
	SetIndexBuffer(m_pCombineIndexBuffer, m_Indices.size());
	m_pVertexShader = m_pCombine.VS.Get();
	m_pInputLayout = m_pCombine.Layout.Get();
	m_pPixelShader = m_pCombinePS.Get();
	RenderPipeline();
	PSSetSRV(21,1,m_pScreenSRV);
	m_pDeviceContext->DrawIndexed(m_Indices.size(),0,0);
	PSSetSRV(21, 1, m_pUISRV);
	m_pDeviceContext->DrawIndexed(m_Indices.size(), 0, 0);
	assert(m_pDrawSwapChain.Get());

	// 프리젠트 꼭 해줘야 함
	HR(m_pDrawSwapChain->Present(0, 0));
}

void DX11Device::ClearDepthView()
{
	m_pDeviceContext->ClearDepthStencilView(m_pScreenDepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDeviceContext->ClearDepthStencilView(m_pUIDepthView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

;
void DX11Device::CreateVS(std::wstring _path, ComPtr<ID3D11VertexShader>* _VS, const std::vector<D3D11_INPUT_ELEMENT_DESC>& _layout, ComPtr<ID3D11InputLayout>* _inputLayout)
{
	HRESULT hr{};

	ID3DBlob* shaderBuffer = nullptr;


	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	//// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	//// Setting this flag improves the shader debugging experience, but still allows 
	//// the shaders to be optimized and to run exactly the way they will run in 
	//// the release configuration of this program.
	//dwShaderFlags |= D3DCOMPILE_DEBUG;
	//// Disable optimizations to further improve shader debugging
	//dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	
#endif


	

	std::wstring path = std::filesystem::current_path();
	path = path.substr(0, path.find(L"5_Project"));
	path += L"4_Resources\\Shader\\";

	std::wstring csoPath = path + L"CSO\\" + _path + L".cso";
#ifdef CompileCSO
#ifdef _DEBUG
	csoPath = L"..\\Libraries\\DebugLib\\Graphics\\" + _path + L".cso";
#else
	csoPath = L"..\\Libraries\\ReleaseLib\\Graphics\\" + _path + L".cso";
#endif
	if (FAILED(hr = D3DReadFileToBlob(csoPath.c_str(), &shaderBuffer)))
#endif
	{
		ID3DBlob* pErrorBlob = nullptr;
		std::wstring hlslPath = L"..\\Graphics\\" + _path + L".hlsl";
		hr = D3DCompileFromFile(hlslPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, dwShaderFlags, &shaderBuffer, &pErrorBlob);
		if (FAILED(hr))
		{
			MessageBox(0, L"Create VS Failed.", 0, 0);
		}
	}

	

	if (_inputLayout->Get() == nullptr)
	{
		hr = m_pDevice->CreateInputLayout(_layout.data(), _layout.size(),
			shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), _inputLayout->GetAddressOf());

	}

	if (_VS->Get() == nullptr)
	{
		hr = m_pDevice->CreateVertexShader(shaderBuffer->GetBufferPointer(),
			shaderBuffer->GetBufferSize(), NULL, _VS->GetAddressOf());

	}
	else
	{
		//assert(false);
	}
	ReleaseCOM(shaderBuffer);
}

void DX11Device::CreatePS(std::wstring _path, ComPtr<ID3D11PixelShader>* _PS)
{
	HRESULT hr = S_OK;

	ID3DBlob* shaderBuffer = nullptr;


	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	//// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	//// Setting this flag improves the shader debugging experience, but still allows 
	//// the shaders to be optimized and to run exactly the way they will run in 
	//// the release configuration of this program.
	//dwShaderFlags |= D3DCOMPILE_DEBUG;
	//// Disable optimizations to further improve shader debugging
	//dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

#else
#endif

	std::wstring path = std::filesystem::current_path();
	path = path.substr(0, path.find(L"5_Project"));
	path += L"4_Resources\\Shader\\";


	std::wstring csoPath = path + L"CSO\\" + _path + L".cso";
#ifdef CompileCSO
#ifdef _DEBUG
	csoPath = L"..\\Libraries\\DebugLib\\Graphics\\" + _path + L".cso";
#else
	csoPath = L"..\\Libraries\\ReleaseLib\\Graphics\\" + _path + L".cso";
#endif
	if (FAILED(hr = D3DReadFileToBlob(csoPath.c_str(), &shaderBuffer)))
#endif // CompileCSO
	{
		ID3DBlob* pErrorBlob = nullptr;
		std::wstring hlslPath = L"..\\Graphics\\" + _path + L".hlsl";
		hr = D3DCompileFromFile(hlslPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, dwShaderFlags, &shaderBuffer, &pErrorBlob);
		if (FAILED(hr))
		{
			MessageBox(0, L"Create PS Failed.", 0, 0);
		}
	}
	
	if (_PS->Get() == nullptr)
	{
		hr = m_pDevice->CreatePixelShader(shaderBuffer->GetBufferPointer(),
			shaderBuffer->GetBufferSize(), NULL, _PS->GetAddressOf());

	}
	
	ReleaseCOM(shaderBuffer);
}

void DX11Device::CreateVertexBuffer(UINT _ByteWidth, const void* _Data, ComPtr<ID3D11Buffer>* _VertexBuffer, bool _IsDynamic)
{
	D3D11_BUFFER_DESC vbd;
	if (_IsDynamic == false)
	{
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.ByteWidth = _ByteWidth;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
	}
	else
	{
		vbd.Usage = D3D11_USAGE_DYNAMIC;
		vbd.ByteWidth = _ByteWidth;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags =D3D11_CPU_ACCESS_WRITE;
		vbd.MiscFlags = 0;
	}

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = _Data;
	if (FAILED(m_pDevice->CreateBuffer(&vbd, &vinitData, _VertexBuffer->GetAddressOf())))
	{
		assert(false);
		// Error
	}

}

void DX11Device::CreateIndexBuffer(UINT _IndexCount, const void* _Data, ComPtr<ID3D11Buffer>* _IndexBuffer)
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = (UINT)(sizeof(UINT) * (_IndexCount));
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = _Data;

	if (FAILED(m_pDevice->CreateBuffer(&ibd, &iinitData, _IndexBuffer->GetAddressOf())))
	{
		assert(false);
		// Error
	}
}

void DX11Device::CreateTexture(const std::string& _Path, ComPtr<ID3D11ShaderResourceView>* _SRV)
{
	std::wstring path;
	path.assign(_Path.begin(),_Path.end());
	if (FAILED(DirectX::CreateDDSTextureFromFile(m_pDevice.Get(), path.c_str(), nullptr, _SRV->GetAddressOf())))
	{
		
		if (FAILED(DirectX::CreateWICTextureFromFile(m_pDevice.Get(), path.c_str(), nullptr, _SRV->GetAddressOf())))
		{
			DirectX::TexMetadata metadata;
			DirectX::ScratchImage image;
			if (FAILED(DirectX::LoadFromTGAFile(path.c_str(), DirectX::TGA_FLAGS_FORCE_SRGB, &metadata, image)))
			{
				DirectX::LoadFromHDRFile(path.c_str(), &metadata, image);
			}
			if (FAILED(DirectX::CreateShaderResourceView(m_pDevice.Get(), image.GetImages(), image.GetImageCount(), metadata, _SRV->GetAddressOf())))
			{
				// error
				//assert(false);
			}
			
		}
		
	}
}

void DX11Device::CreateTexture(const std::wstring& _Path, ComPtr<ID3D11ShaderResourceView>* _SRV)
{

	if (FAILED(DirectX::CreateDDSTextureFromFile(m_pDevice.Get(), _Path.c_str(), nullptr, _SRV->GetAddressOf())))
	{

		if (FAILED(DirectX::CreateWICTextureFromFile(m_pDevice.Get(), _Path.c_str(), nullptr, _SRV->GetAddressOf())))
		{
			DirectX::TexMetadata metadata;
			DirectX::ScratchImage image;
			if (FAILED(DirectX::LoadFromTGAFile(_Path.c_str(), DirectX::TGA_FLAGS_FORCE_SRGB, &metadata, image)))
			{
				DirectX::LoadFromHDRFile(_Path.c_str(), &metadata, image);
			}
			if (FAILED(DirectX::CreateShaderResourceView(m_pDevice.Get(), image.GetImages(), image.GetImageCount(), metadata, _SRV->GetAddressOf())))
			{
				MessageBox(NULL, L"Path Error", L"CreateTexture Error", NULL);
			}

		}

	}
}
