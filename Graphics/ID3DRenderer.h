#pragma once
#include "RendererDef.h"

class ID3DRenderer abstract
{
public:
	ID3DRenderer();
	virtual ~ID3DRenderer();

	static ID3DRenderer* CreateRenderer();

#pragma region RenderFoo
	// 기본 인터페이스

	virtual bool Init(int hinst, int hWnd, int screenHeight, int screenWidth, float _GameScreenSize) abstract;
	virtual void FixedUpdate() abstract;
	virtual void Update(double _DeltaTime) abstract;
	virtual void Render() abstract;
	virtual void BeginRender() abstract;
	virtual void EndRender() abstract;
	virtual void Finalize() abstract;

	// 창 변환 관련
	virtual void SetClientSize(int width, int height) abstract;
	virtual Vector2 GetClientSize() abstract;
	virtual void OnResize() abstract;
	virtual void OnResize(int screenWidth, int screenHeight) abstract;
	virtual bool IsValidDevice() abstract;
	virtual void SetWinMinMax(bool isMinimized, bool isMaxmized) abstract;

	// Mouse Position Input
	virtual void SetMousePosition(long x, long y) abstract;

	// Send Massege to GraphicsEngine
	virtual void Send(UINT _MessageNum, UINT _ObjectID, double _Data, Vector4 _Color) abstract;

#pragma endregion

	/// <summary>
	/// </summary>
	/// <param name="_CameraID"> 0~7 </param>
	/// <param name="_ParentID"> 0 is not have parent </param>
	/// <param name="_transform"> LocalTransform </param>
	virtual void CreateCamera(UINT _CameraID, RenderTransform* _transform, RenderMatrix44* _View) abstract;

	/// <summary>
	/// </summary>
	/// <param name="_Fov"> is Degree</param>
	/// <param name="_Near"> more than 0.1f </param>
	/// <param name="_Far"></param>
	/// <returns></returns>
	virtual bool SetCameraProjection(float _Fov,float _AspectRatio,float _Near, float _Far, UINT _CameraID) abstract;
	virtual bool SetCameraProjection(float _Fov, float _Near, float _Far, UINT _CameraID) abstract;
	virtual bool SetCameraProjection(float _Fov, UINT _CameraID) abstract;
	virtual RenderMatrix44* GetCameraProjection(UINT _CameraID) abstract;

	virtual void SetDirectionLight(Vector3* _Direction, float* _Power) abstract;
	virtual void SetDirectionLight(Vector3 _Direction, float _Power) abstract;
#pragma region LoadAsset

	virtual void LoadModel(const char* _Path,UINT _AssetNum) abstract;
	virtual void LoadSprite(UINT _AssetNum) abstract;
	virtual void LoadTexture(const char* _Path) abstract;
	// 미작성
	virtual void LoadGameEffect(const char* _Path, UINT _AssetNum) abstract;
	virtual void AddTextureToSprite(const char* _Path, UINT _AssetNum, int _TextureIndex) abstract;

	virtual void CreateModel(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, UINT* _State, bool* _IsRender) abstract;
	virtual void CreateSprite(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform,Vector2* _Size, UINT* _State, bool* _IsRender) abstract;
	virtual void CreateEffect(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, UINT* _State, bool* _IsRender) abstract;

	virtual void CreateModel(UINT* _AssetNum, UINT _ObjectID, void* _Transform, UINT* _State, bool* _IsRender) abstract;
	virtual void CreateSprite(UINT* _AssetNum, UINT _ObjectID, void* _Transform, Vector2* _Size, UINT* _State, bool* _IsRender) abstract;
	virtual void CreateEffect(UINT* _AssetNum, UINT _ObjectID, void* _Transform, UINT* _State, bool* _IsRender) abstract;

	virtual void DestroyAllObject() abstract;
#pragma endregion

	virtual void DrawDebugingBoundingBox(Vector3 _Center, Vector3 _Extents, Vector4 _Orientation, Vector4 _Color) abstract;
	virtual void DrawDebugingBoundingSphere(Vector3 _Center, float _Radius, Vector4 _Color) abstract;
	virtual void DrawDebugingRay(Vector4 _Origin, Vector4 _Direction, Vector4 _Color, bool _IsNormal = false) abstract;

	// for animation
	virtual double GetCurrentAnimationTime(UINT _ObjectID) abstract;
	virtual AnimationInfo GetAnimationInfo(UINT _AssetNum) abstract;

	virtual void PlayAnimation(UINT _ObjectID) abstract;
	virtual void StopAnimation(UINT _ObjectID) abstract;
	virtual void SetAnimationTime(UINT _ObjectID,double _time = 0) abstract;

	virtual void SetAnimationSpeed(UINT _ObjectID, float _Speed) abstract;
	virtual void SetAnimationRepeat(UINT _ObjectID, bool _Value) abstract;
	virtual bool GetAnimationComplete(UINT _ObjectID) abstract;




};