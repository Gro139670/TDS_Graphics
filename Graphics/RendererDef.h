#pragma once

#pragma region RendererDef
using UINT = unsigned int;

struct Vector2 { float x{}, y{}; };
struct Vector3 { float x{}, y{}, z{}; };
struct Vector4 { float x{}, y{}, z{}, w{}; };

struct RenderTransform { Vector3 Position, Rotation, Scale; };
struct RenderMatrix44 { Vector4 m1, m2, m3, m4; };

struct Animations
{
	const char* AnimationName;
	UINT Duration;
	// 애니메이션이 재생되는 전체시간
	double AnimationFullTime = 0;
	// 1프레임에 걸리는 시간
	double TickPerFrame = 0;
};

struct AnimationInfo
{
	// Max Animation Size
	UINT AnimationNum;
	// 일단 크게 잡는다.
	Animations Animation[100];
};


#pragma endregion