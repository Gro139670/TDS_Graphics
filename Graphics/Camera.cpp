#include "pch.h"
#include "Camera.h"

void Camera::SetCameraTransform(RenderTransform* _Transform, RenderMatrix44* _View)
{
	m_pTransform = _Transform;
	m_pView = _View;
}

void Camera::CameraMove()
{
	memcpy(&m_View,m_pView,sizeof(SimpleMath::Matrix));
	SimpleMath::Vector3 s;
	SimpleMath::Quaternion q;
	memcpy(&m_Position, &m_pTransform->Position, sizeof(SimpleMath::Vector3));
}
