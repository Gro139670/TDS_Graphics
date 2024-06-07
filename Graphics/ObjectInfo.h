#pragma once
#include <string>

/// 이거 때문에 렌더러 인터페이스에 d3d들어가니까 쓰지말고 직접 만든 math헤더 사용해
#include "../Libraries/DXTK/Inc/SimpleMath.h"

/// <summary>
/// 2024.01.24 - 강규석
/// LightType을 위해 만들었다.
/// </summary>
enum class LightType
{
	Directional,
	Spot,
	Point,
};

/// <summary>
/// 빛 ID, 빛 타입, 
/// </summary>
struct LightInfo
{
	size_t _lightID;
	// 타입은 enum으로 바꿀예정
	LightType _lightType;
	size_t _intensity = 1;

	DirectX::SimpleMath::Vector4 Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::SimpleMath::Vector3 Attenuation = { 0.f, 0.f, 1.f };
	float _range = 10.f;
};

/// <summary>
/// 오브젝트 ID, 파일경로, 셰이더경로 구조체
/// </summary>
struct MeshObjectInfo
{
	size_t _objectID;

	const char* _meshPath;

	std::wstring _texturePath;

	std::wstring _shaderPath;

};


