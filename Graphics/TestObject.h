#pragma once
#include "Object.h"

class TestObject : public StaticObject
{
public:
	TestObject(ComPtr<ID3D11Device> _device);
	~TestObject() {};

public:


public:
private:


	UINT boxIndices[36];
};




