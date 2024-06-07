#pragma once
#include "DRCamera.h"
#include "DX11Device.h"

class IObject
{
public:
	IObject() {};
	virtual ~IObject() {};

	virtual void Initialize(ID3D11Device* _device) abstract;
	virtual void Update() abstract;
	virtual void Render() abstract;

};

