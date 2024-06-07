#pragma once
#include "framework.h"

class Camera
{
public:
	Camera() {}
	~Camera() {}

public:
	void SetCameraTransform(RenderTransform* _Transform, RenderMatrix44* _View);

	void CameraMove();

	const SimpleMath::Vector3& GetPosition() { return m_Position; }
	const SimpleMath::Vector3& GetRotation() { return m_Rotation; }

	const SimpleMath::Matrix& GetView() { return m_View; }
	const SimpleMath::Matrix& GetProjection() { return m_Projection; }


public:
	SimpleMath::Matrix m_Projection{};
	float m_NEAR = 1, m_FAR = 10;
private:
	RenderTransform* m_pTransform = nullptr;
	RenderMatrix44* m_pView = nullptr;
	SimpleMath::Matrix m_View{};


	SimpleMath::Vector3 m_Position{};
	SimpleMath::Vector3 m_Rotation{};
	SimpleMath::Quaternion m_Quaternion{};
private:


	SimpleMath::Vector3 m_Forward{};
	SimpleMath::Vector3 m_Right{};
	SimpleMath::Vector3 m_UP{};

	SimpleMath::Matrix m_Matrix{};


};

