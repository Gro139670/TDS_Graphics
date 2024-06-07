#include "pch.h"
#include "DXTKFont.h"
#include <tchar.h>

#define SafeDelete(x) { delete x; x = 0; }
#define SAFE_RELEASE(x) { if(x){ x->Release(); x = 0; } }	// 예전 스타일

//#include <DirectXMath.h> // XMFLOAT
#include "../Libraries/DXTK/Inc/SimpleMath.h"

DXTKFont::DXTKFont()
	: m_pSpriteBatch(nullptr), m_pSpriteFont(m_pSpriteFont), m_RasterizerState(nullptr), m_DepthStencilState(nullptr)
{

}

DXTKFont::~DXTKFont()
{
	SafeDelete(m_pSpriteFont);
	SafeDelete(m_pSpriteBatch);
}

void DXTKFont::Create(ID3D11Device* pDevice, ID3D11RasterizerState* rs, ID3D11DepthStencilState* ds)
{
	ID3D11DeviceContext* pDC = nullptr;
	pDevice->GetImmediateContext(&pDC);

	m_pSpriteBatch = new DirectX::SpriteBatch(pDC);

	TCHAR* _fileName = (TCHAR*)L"../Resources/Font/gulim9k.spritefont";

	m_pSpriteFont = new DirectX::SpriteFont(pDevice, _fileName);
	m_pSpriteFont->SetLineSpacing(14.0f);
	//m_pSpriteFont->SetDefaultCharacter(' ');

	SAFE_RELEASE(pDC);

	m_RasterizerState = rs;
	m_DepthStencilState = ds;
}

void DXTKFont::DrawTextColor(int x, int y, DirectX::XMFLOAT4 color, const wchar_t* text, ...)
{
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, text, DirectX::XMFLOAT2((float)x, (float)y), DirectX::SimpleMath::Vector4(color));
	m_pSpriteBatch->End();
}
