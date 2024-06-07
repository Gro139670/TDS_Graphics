#include "pch.h"
#include "MeshObject.h"

// MeshObject::MeshObject(GeometryBuffer* buffer, Shader* shader, Texture* texture)
// {
// 
// }
// 
// MeshObject::~MeshObject()
// {
// 
// }
// 
// void MeshObject::Update(DRCamera* camera)
// {
// 	XMStoreFloat4x4(&m_View, camera->View());
// 	XMStoreFloat4x4(&m_Proj, camera->Proj());
// 
// 	m_EyePosW = XMFLOAT3(camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
// }
// 
// void MeshObject::Render(DX11Device* device)
// {
// 	ID3D11DeviceContext* m_DeviceContext = device->GetD3DDeviceContext();
// 	ID3D11RasterizerState* m_Renderstate = device->GetSolidRS();
// 
// 	/// TODO : 셰이더에서 인풋레이아웃 Get해야함
// 	m_DeviceContext->IASetInputLayout(m_Shader->GetInputLayout());
// 	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
// 
// 	/*m_DeviceContext->VSSetShader();*/
// 
// 	// 렌더 스테이트
// 	m_DeviceContext->RSSetState(m_Renderstate);
// 
// 	// 버텍스버퍼와 인덱스버퍼 셋팅
// 	UINT offset = 0;
// 
// 	/// delete : 잠시 냅둠 지울것
// // 	m_Shader->SetDirLight(m_DirLight);
// // 	m_Shader->SetPointLight(m_PointLight);
// // 	m_Shader->SetSpotLight(m_SpotLight);
// // 
// // 	m_Shader->SetEyePosW(m_EyePosW);
// 
// 	//for (UINT p = 0; p < m_Buffer->GetVBSize(); ++p)
// 	{
// 		m_DeviceContext->IASetVertexBuffers(0, 1, &m_Buffer->m_VBs, &m_Buffer->m_Stride, &offset);
// 		m_DeviceContext->IASetIndexBuffer(m_Buffer->m_IBs, DXGI_FORMAT_R32_UINT, 0);
// 
// 		XMMATRIX world = XMLoadFloat4x4(&m_World);
// 		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
// 		XMMATRIX worldViewProj = world * m_View * m_Proj;
// 
// 		m_Shader->SetWorld(world);
// 		m_Shader->SetWorldInvTranspose(worldInvTranspose);
// 		m_Shader->SetWorldViewProj(worldViewProj);
// // 		m_Shader->SetTexTransform(XMLoadFloat4x4(&m_TexTransform));
// // 		m_Shader->SetMaterial(m_Material);
// 
// // 		if (m_Texture)
// // 		{
// // 			m_Shader->SetDiffuseMap(m_Texture->GetTextureSRV());
// // 		}
// 
// 		//m_Shader->LightTexTech->GetPassByIndex(p)->Apply(0, m_DeviceContext);
// 		m_DeviceContext->DrawIndexed(m_Buffer->GetIndexCount(), 0, 0);
// 	}
// }
// 
// void MeshObject::SetTransformMatrix(DirectX::XMMATRIX transformMatrix)
// {
// 	XMStoreFloat4x4(&m_World, transformMatrix);
// }
