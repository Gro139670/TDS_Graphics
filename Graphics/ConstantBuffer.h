#pragma once
#include "framework.h"

#pragma region ConstantBuffer
namespace CB
{
	struct CameraBuffer
	{
		SimpleMath::Matrix View;
		SimpleMath::Vector3 CameraPosition;
		float PlayerHP;

		SimpleMath::Color Color{1.f,1.f,1.f,1.f};
		// 그림자 표현 안하거나 라이트는 카메라를 따라다닌다.
		//SimpleMath::Matrix ShadowView;
	};


	struct ObjectBuffer
	{
		SimpleMath::Matrix WorldTransform;

		// 우리는 아트가 없으므로 이 두가지 값은 쓰지 않겠다.
		//float Metallic;
		//float Roughness;

		SimpleMath::Vector4 ObjectColor;

		SimpleMath::Vector4 ObjectBaseColor;

		float Metallic;
		float Loughness;

		SimpleMath::Vector2 Flag;
	};

	struct BoneBuffer
	{
		std::array<SimpleMath::Matrix,120> BoneTransform;
	};

	struct EngineBuffer
	{
		// 그림자 표현 안하거나 라이트는 카메라를 따라다닌다.
		// SimpleMath::Matrix ShadowProjection;
		SimpleMath::Matrix Projection;
		SimpleMath::Matrix SpriteProjection;
	};

	struct DirectionLightBuffer
	{
		SimpleMath::Vector3 WorldDirectionLight;
		float WorldDirectionPower{};
	};
}
#pragma endregion