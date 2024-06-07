#include "pch.h"
#include "ID3DRenderer.h"
#include "D3DRenderer.h"


ID3DRenderer::ID3DRenderer()
{
}

ID3DRenderer::~ID3DRenderer()
{
}

ID3DRenderer* ID3DRenderer::CreateRenderer()
{
	D3DRenderer* renderer = new D3DRenderer;
	return renderer;
}
