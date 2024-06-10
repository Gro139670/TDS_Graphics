#pragma once
#include "Object.h"



/// <summary>
/// 240205
/// [동휘]측에서 어심프 데이터를 파싱과 구조화를 240212까지 마치지 못할것으로 예상하는 바.
/// [용준]측에서 데이터 구조를 정의한다.
/// (만드는 중....)
/// </summary>
namespace Yongs
{
}

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




