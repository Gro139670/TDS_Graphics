#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

// cpp외에 헤더에서도 필요한 stl들을 추가한다.
// 사용할 헤더에 이 프레임워크 헤더를 추가해 사용한다.

#include "RendererDef.h"

#include <windows.h>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <memory>
#include <d3d11.h>
#include <dxgi.h>

#include "..\Libraries\DXTK\Inc\SimpleMath.h"
using namespace DirectX;

#include <wrl\client.h>
using namespace Microsoft;
using namespace WRL;


#pragma comment (lib, "d3d11.lib")
#pragma comment (lib,"dxgi.lib")

/// <summary>
/// 이렇게 받는다. 머티리얼정보는 없다고 가정한다.
/// 240205
/// -> 머티리얼 정보는 엔진측에서 변화시키지 않는다고 가정한다.
/// 240206
/// 렌더링 오브젝트를 관리해주는 함수를 만들어야한다.(에셋지우기 등등);
/// </summary>


/// <summary>
/// 원래는 오브젝트별로 구조체를 따로 해야 효율적이지만 일단 하나에 다 만든다.
/// </summary>

struct ObjectInfo
{
	// 다용도로 쓰일 변수
	double m_Data = 0;
	float m_Metallic = 0;
	float m_Loughness = 0;
	bool m_IsLateRender = false;



	UINT m_ID = 0;
	RenderTransform* m_pWorldTM = nullptr;
	SimpleMath::Matrix* m_pWorldTransform = nullptr;
	UINT* m_pSTATE = nullptr;
	bool* m_pIsRender = nullptr;
	UINT m_CurrentSTATE = UINT32_MAX;
	SimpleMath::Color m_ColorPower{ 0.f,0.f,0.f,0.f };
	SimpleMath::Color m_BaseColor{0.f,0.f,0.f,0.f};


	// for sprite
	Vector2* m_pSize = nullptr;
	bool m_IsOriginCenter = false;

	// for Animation
	double m_CurrentTime = 0;
	UINT m_CurrentFrame = 0;
	float m_AnimationSpeed = 1.f;
	bool m_IsAnimationRefeat = false;
	bool m_IsAnimationPlay = true;
	bool m_IsAnimationComplete = false;

	// 땜빵코드
	bool m_ForBox = false;




	};
