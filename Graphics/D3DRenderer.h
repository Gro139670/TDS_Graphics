#pragma once
#include "framework.h"
#include "ID3DRenderer.h"
#include "DX11Define.h"
#include "ConstantBuffer.h"
#include "DX11Device.h"
#include <array>

constexpr int CameraNum = 8;

#pragma region _FORWARD_DEFINITION
class DXTKFont;
class TestObject;
class Converter;
namespace Render
{
	class ObjectManager;
	class ResourceManager;
}
#pragma endregion

class Camera;
class D3DRenderer final :public ID3DRenderer
{
#pragma region Function
public:
	D3DRenderer();
	virtual ~D3DRenderer();

	// 기본 인터페이스
	bool Init(int hinst, int hWnd, int screenHeight, int screenWidth, float _GameScreenSize) override final;
	void FixedUpdate() override;
	void Update(double _DeltaTime) override final;
	void BeginRender() override final;
	void Render() override final;
	void EndRender() override final;
	void Finalize() override final;

	// 창 변환 관련
	void SetClientSize(int width, int height) override final;
	Vector2 GetClientSize() override final;
	void OnResize() override final;
	void OnResize(int screenWidth, int screenHeight) override final;
	bool IsValidDevice() override final;
	void SetWinMinMax(bool isMinimized, bool isMaxmized) override final;

	void SetMousePosition(long x, long y) override;

	void Send(UINT _MessageNum, UINT _ObjectID, double _Data, Vector4 _Color ) override;

	// Camera
	bool SetCameraProjection(float _Fov, float _AspectRatio, float _Near, float _Far, UINT _CameraID) override;
	bool SetCameraProjection(float _Fov, float _Near, float _Far, UINT _CameraID) override;
	bool SetCameraProjection(float _Fov, UINT _CameraID) override;
	RenderMatrix44* GetCameraProjection(UINT _CameraID) override;

	void SetDirectionLight(Vector3* _Direction, float* _Power) override;
	void SetDirectionLight(Vector3 _Direction, float _Power) override;


	void CreateCamera(UINT _CameraID, RenderTransform* _Transform, RenderMatrix44* _View) override;

	// Load Asset

	void LoadModel(const char* _Path, UINT _AssetNum) override;
	void LoadTexture(const char* _Path);
	void LoadSprite(UINT _AssetNum) override;
	void LoadGameEffect(const char* _Path, UINT _AssetNum) override;
	void AddTextureToSprite(const char* _Path, UINT _AssetNum, int _TextureIndex) override;

	void CreateModel(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, UINT* _State, bool* _IsRender) override;
	void CreateSprite(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, Vector2* _Size, UINT* _State, bool* _IsRender) override;
	void CreateEffect(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, UINT* _State, bool* _IsRender) override;

	void CreateModel(UINT* _AssetNum, UINT _ObjectID, void* _Transform, UINT* _State, bool* _IsRender) override;
	void CreateSprite(UINT* _AssetNum, UINT _ObjectID, void* _Transform, Vector2* _Size, UINT* _State, bool* _IsRender) override;
	void CreateEffect(UINT* _AssetNum, UINT _ObjectID, void* _Transform, UINT* _State, bool* _IsRender) override;

	void DestroyObject(UINT _ObjectID);
	void DestroyAllObject() override;

	// todo : define

	void DrawDebugingBoundingBox(Vector3 _Center, Vector3 _Extents, Vector4 _Orientation, Vector4 _Color) override;
	void DrawDebugingBoundingSphere(Vector3 _Center, float _Radius, Vector4 _Color) override;

	void DrawDebugingRay(Vector4 _Origin, Vector4 _Direction, Vector4 _Color, bool _IsNormal = false) override;

	void PlayAnimation(UINT _ObjectID) override;
	void StopAnimation(UINT _ObjectID) override;
	void SetAnimationTime(UINT _ObjectID, double _time = 0) override;

	void SetAnimationSpeed(UINT _ObjectID, float _Speed) override;
	void SetAnimationRepeat(UINT _ObjectID, bool _Value) override;
	bool GetAnimationComplete(UINT _ObjectID) override;

	double GetCurrentAnimationTime(UINT _ObjectID) override;
	AnimationInfo GetAnimationInfo(UINT _AssetNum) override;

	void UIOriginSetCenter() {};
	void UIOriginSetLeftTop() {};

private:
	ObjectInfo* FindObject(UINT _ObjectID);

	//void FastRender();
	void NormalRender();
	void LateRender();

	void DrawStatic();
	void DrawSkeletal();
	void AnimationUpdate(double _DeltaTime);
	void DrawSprite();

private:
	float GetAspectRatio() const;
#pragma endregion
private:

	bool m_IsFastRender = false;
	bool m_IsLateRender = false;

	//------------------------------------------------------
	HWND m_hWnd;
	int m_ScreenWidth;
	int m_ScreenHeight;
	bool m_AppPaused;
	bool m_Minimized;
	bool m_Maximized;

	///------------------------------------------------------------------------------------------
	/// DirectX 11 관련 멤버

	// DX11 Device
	std::unique_ptr<DX11Device> m_pDevice;

private:
	std::unique_ptr<Render::ObjectManager> m_pObjectManager;
	std::shared_ptr<Render::ResourceManager> m_pResourceManager;
	std::unique_ptr<Converter> m_pFBXParser;
	// 시간 관련
	int m_TotalFrameCount; // 렌더러가 한 번 업데이트-그려졌을 때 카운트 올림
	double m_PlayerHP = 0;

private:
	// DXTKFont

private:
	// Camera
	std::unique_ptr<Camera> m_pCamera[CameraNum];
	POINT m_LastMousePos;
	RenderMatrix44 m_CameraProjection;


	CB::CameraBuffer m_CameraBuffer[CameraNum];
	CB::ObjectBuffer m_ObjectBuffer;
	// 일단 10개로 잡아본다.
	std::array<CB::BoneBuffer,10> m_BoneBuffer;
	CB::EngineBuffer m_EngineBuffer;
	CB::DirectionLightBuffer m_DirectionLightBuffer;


	struct DebugBox
	{
		DirectX::BoundingOrientedBox _Box;
		SimpleMath::Vector4 _Color;
	};
	struct DebugSphere
	{
		DirectX::BoundingSphere _Sphere;
		SimpleMath::Vector4 _Color;
	};
	struct DebugRay
	{
		SimpleMath::Vector4 _Origin, _Direction, _Color;
		bool _IsNormal;
	};
	std::vector<DebugBox> m_DebugBox;
	std::vector<DebugSphere> m_DebugSphere;
	std::vector<DebugRay> m_DebugRay;

	// interface Convert
	SimpleMath::Vector3 m_Pos, m_Rot, m_Scale;
	// interface Convert
	SimpleMath::Matrix m_World;

	Vector3* m_pDirLight = nullptr;
	float* m_pDirLightPower = nullptr;
};


inline void D3DRenderer::SetClientSize(int width, int height)
{
	m_ScreenWidth = width;
	m_ScreenHeight = height;
}

inline Vector2 D3DRenderer::GetClientSize()
{
	Vector2 result;
	result.x = m_ScreenWidth;
	result.y = m_ScreenHeight;

	return result;
}

// 창 변환 관련
inline void D3DRenderer::OnResize()
{
	m_pDevice->OnResize(m_ScreenWidth, m_ScreenHeight);
}

inline void D3DRenderer::OnResize(int screenWidth, int screenHeight)
{
	m_pDevice->OnResize(screenWidth, screenHeight);
}

inline bool D3DRenderer::IsValidDevice()
{
	return (m_pDevice->GetDevice() != nullptr);
}

inline void D3DRenderer::SetWinMinMax(bool isMinimized, bool isMaxmized)
{
	m_Minimized = isMinimized;
	m_Maximized = isMaxmized;
}

inline void D3DRenderer::SetMousePosition(long x, long y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

}
