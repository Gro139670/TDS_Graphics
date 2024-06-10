#include "pch.h"

#include "Camera.h"
#include "D3DRenderer.h"
#include "DX11Device.h"
#include "FBXParser.h"

//#include "D3DHelper.h"

//#include "DXTKFont.h"

#include "DebugDraw.h"
#include <functional>


D3DRenderer::D3DRenderer()
	: m_hWnd(0), m_ScreenWidth(800), m_ScreenHeight(600)
	, m_AppPaused(false), m_Minimized(false), m_Maximized(false)
	, m_TotalFrameCount(0)
{
}

D3DRenderer::~D3DRenderer()
{
}

bool D3DRenderer::Init(int hinst, int hWnd, int screenHeight, int screenWidth, float _GameScreenSize)
{
	bool result = true;
	/// 윈도 정보
	m_hWnd = (HWND)hWnd;
	m_ScreenWidth = screenWidth;
	m_ScreenHeight = screenHeight;

	m_pDevice = std::make_unique<DX11Device>();
	m_pDevice->Create(m_hWnd, m_ScreenWidth, m_ScreenHeight, _GameScreenSize);

	m_pDevice->CreateConstantBuffer<CB::CameraBuffer>((UINT)CB_Type::CameraBuffer);
	m_pDevice->CreateConstantBuffer<CB::EngineBuffer>((UINT)CB_Type::EngineBuffer);
	m_pDevice->CreateConstantBuffer<CB::ObjectBuffer>((UINT)CB_Type::ObjectBuffer);
	m_pDevice->CreateConstantBuffer<CB::DirectionLightBuffer>((UINT)CB_Type::DirectionLightBuffer);
	m_pDevice->CreateConstantBuffer<CB::BoneBuffer>((UINT)CB_Type::BoneBuffer);


	m_pObjectManager = std::make_unique<Render::ObjectManager>();
	m_pResourceManager = std::make_shared<Render::ResourceManager>();
	m_pFBXParser = std::make_unique<Converter>();
	m_pFBXParser->SetResourceManager(m_pResourceManager);


	m_DirectionLightBuffer.WorldDirectionLight = { 1,0,0 };
	m_DirectionLightBuffer.WorldDirectionPower = 1;

	m_pDevice->SetConstantBuffer(CB_Type::DirectionLightBuffer, &m_DirectionLightBuffer, Shader_Type::PixelShader);

	DebugDraw::Initialize(m_pDevice->GetDevice(), m_pDevice->GetDeviceContext());

	m_EngineBuffer.SpriteProjection = DirectX::XMMatrixOrthographicLH(m_ScreenWidth, m_ScreenHeight,0.f,1000.f);


	return result;
}

void D3DRenderer::FixedUpdate()
{
	if (m_pDirLight != nullptr && m_pDirLightPower != nullptr)
	{
		memcpy(&m_DirectionLightBuffer.WorldDirectionLight, m_pDirLight, sizeof(SimpleMath::Vector3));
		m_DirectionLightBuffer.WorldDirectionPower = *m_pDirLightPower;
	}
	m_pDevice->SetConstantBuffer(CB_Type::DirectionLightBuffer, &m_DirectionLightBuffer, Shader_Type::PixelShader);
}

void D3DRenderer::Update(double _DeltaTime)
{

	AnimationUpdate(_DeltaTime);
	

}

void D3DRenderer::BeginRender()
{
	m_pDevice->OnBeginRender();



	if (m_pCamera[0] != nullptr)
		m_pCamera[0]->CameraMove();

	m_CameraBuffer[0].View = m_pCamera[0]->GetView().Transpose();
	m_CameraBuffer[0].CameraPosition = m_pCamera[0]->GetPosition();
	m_pDevice->SetConstantBuffer(CB_Type::CameraBuffer, &m_CameraBuffer[0], Shader_Type::All);

	
	
}

void D3DRenderer::Render()
{
	//FastRender();
	NormalRender();
	LateRender();

	
}

void D3DRenderer::EndRender()
{

	m_pDevice->GetDeviceContext()->OMSetBlendState(DebugDraw::g_States->Opaque(), nullptr, 0xFFFFFFFF);
	DebugDraw::g_BatchEffect->Apply(m_pDevice->GetDeviceContext().Get());
	DebugDraw::g_BatchEffect->SetView(m_CameraBuffer[0].View.Transpose());
	DebugDraw::g_BatchEffect->SetProjection(m_EngineBuffer.Projection.Transpose());
	m_pDevice->GetDeviceContext()->IASetInputLayout(DebugDraw::g_pBatchInputLayout.Get());

	DebugDraw::g_Batch->Begin();

	for (auto& box : m_DebugBox)
	{
		DebugDraw::Draw(DebugDraw::g_Batch.get(), box._Box, box._Color);
	}
	m_DebugBox.clear();
	DebugDraw::g_Batch->End();
	/*DebugDraw::g_Batch->Begin();

	for (auto& sphere : m_DebugSphere)
	{
		DebugDraw::Draw(DebugDraw::g_Batch.get(), sphere._Sphere, sphere._Color);
	}
	m_DebugSphere.clear();
	DebugDraw::g_Batch->End();
	DebugDraw::g_Batch->Begin();

	for (auto& ray : m_DebugRay)
	{
		DebugDraw::DrawRay(DebugDraw::g_Batch.get(), ray._Origin,ray._Direction, ray._IsNormal, ray._Color);
	}
	m_DebugRay.clear();
	DebugDraw::g_Batch->End();*/
	//OnResize();
	DrawSprite();

	m_pDevice->GetDeviceContext()->OMSetBlendState(nullptr, NULL, 0xffffffff);
	m_pDevice->OnEndRender();



	


}



void D3DRenderer::Send(UINT _MessageNum, UINT _ObjectID, double _Data, Vector4 _Color)
{
	ObjectInfo* info;
	switch (_MessageNum)
	{
		// PlayerHP
		
	case 0:
		m_CameraBuffer[0].PlayerHP = _Data;
		m_CameraBuffer[0].Color = { _Color.x,_Color.y,_Color.z,static_cast<float>(_Data) };
		break;

	case 1:
		info = FindObject(_ObjectID);
		info->m_ColorPower = { _Color.x,_Color.y,_Color.z, static_cast<float>(_Data)};
		break;

		// late Render
	case 2:
		info = FindObject(_ObjectID);
		if (_Data > 0.5f)
		{
			info->m_IsLateRender = true;
		}
		else
		{
			info->m_IsLateRender = false;
		}
		break;

		// chaser color
	case 3:
		info = FindObject(_ObjectID);
		info->m_BaseColor = { _Color.x,_Color.y,_Color.z,static_cast<float>(_Data) };

		break;
		// ===================================================================
		// 여기부터 땜빵코드
	case 4:
		break;
	case 5:

		// ===================================================================
		// metallic
	case 11:
		FindObject(_ObjectID)->m_Metallic = _Data;
		break;

		// loughness
	case 12:
		FindObject(_ObjectID)->m_Loughness = _Data;
		break;
	default:
		break;
	}
}



bool D3DRenderer::SetCameraProjection(float _Fov, float _AspectRatio, float _Near, float _Far, UINT _CameraID)
{
	if (m_pCamera[_CameraID].get() == nullptr) return false;
	if (_Near < 0.1f)
	{
		_Near = 0.1f;
	}
	m_pCamera[_CameraID]->m_NEAR = _Near;
	m_pCamera[_CameraID]->m_FAR = _Far;

	m_pCamera[_CameraID]->m_Projection = DirectX::XMMatrixPerspectiveFovLH(_Fov * 3.141592/180, _AspectRatio, _Near, _Far);

	//m_pCamera[_CameraID]->m_Projection *= SimpleMath::Matrix::CreateScale(0.3334f,0.3334f,0.3334f);
	//m_pCamera[_CameraID]->m_Projection *= SimpleMath::Matrix::CreateScale(2, 2, 2);

	// s나중에 바꿔야할 부분.
	m_EngineBuffer.Projection = m_pCamera[_CameraID]->m_Projection.Transpose();
	m_pDevice->SetConstantBuffer(CB_Type::EngineBuffer, &m_EngineBuffer, Shader_Type::All);

	return true;
}

bool D3DRenderer::SetCameraProjection(float _Fov, float _Near, float _Far, UINT _CameraID)
{
	if (m_pCamera[_CameraID].get() == nullptr) return false;
	if (_Near < 0.1f)
	{
		_Near = 0.1f;
	}
	m_pCamera[_CameraID]->m_NEAR = _Near;
	m_pCamera[_CameraID]->m_FAR = _Far;

	m_pCamera[_CameraID]->m_Projection = DirectX::XMMatrixPerspectiveFovLH(_Fov * 3.141592 / 180, GetAspectRatio(), _Near, _Far);

	//m_pCamera[_CameraID]->m_Projection *= SimpleMath::Matrix::CreateScale(0.3334f, 0.3334f, 0.3334f);
	//m_pCamera[_CameraID]->m_Projection *= SimpleMath::Matrix::CreateScale(2, 2, 2);


	// s나중에 바꿔야할 부분.
	m_EngineBuffer.Projection = m_pCamera[_CameraID]->m_Projection.Transpose();
	m_pDevice->SetConstantBuffer(CB_Type::EngineBuffer, &m_EngineBuffer, Shader_Type::All);

	return true;
}

bool D3DRenderer::SetCameraProjection(float _Fov, UINT _CameraID)
{
	m_pCamera[_CameraID]->m_Projection = DirectX::XMMatrixPerspectiveFovLH(_Fov *3.141592/180, GetAspectRatio(), m_pCamera[_CameraID]->m_NEAR, m_pCamera[_CameraID]->m_FAR);

	//m_pCamera[_CameraID]->m_Projection *= SimpleMath::Matrix::CreateScale(0.3334f, 0.3334f, 0.3334f);
	//m_pCamera[_CameraID]->m_Projection *= SimpleMath::Matrix::CreateScale(2, 2, 2);

	m_EngineBuffer.Projection = m_pCamera[_CameraID]->m_Projection.Transpose();
	m_pDevice->SetConstantBuffer(CB_Type::EngineBuffer, &m_EngineBuffer, Shader_Type::All);
	return true;
}

RenderMatrix44* D3DRenderer::GetCameraProjection(UINT _CameraID)
{
	memcpy(&m_CameraProjection, &m_pCamera[_CameraID]->m_Projection, sizeof(SimpleMath::Matrix));
	return &m_CameraProjection;
}

void D3DRenderer::SetDirectionLight(Vector3* _Direction, float* _Power)
{
	m_pDirLight = _Direction;
	m_pDirLightPower = _Power;
	
}

void D3DRenderer::SetDirectionLight(Vector3 _Direction, float _Power)
{
	memcpy(&m_DirectionLightBuffer.WorldDirectionLight, &_Direction, sizeof(SimpleMath::Vector3));
	m_DirectionLightBuffer.WorldDirectionPower = _Power;
}

void D3DRenderer::CreateCamera(UINT _CameraID, RenderTransform* _Transform, RenderMatrix44* _View)
{
#ifdef _DEBUG
#endif
	if (_Transform == nullptr || _CameraID+1 > 8)
	{
		return;
	}

	m_pCamera[_CameraID] = std::make_unique<Camera>();
	SetCameraProjection(45,16.f/9.f, 1.f, 20000.f, _CameraID);
	// 프로젝션 행렬 생성 및 세팅(엔진 내에서 거의 바뀌지 않는다.)

	m_pCamera[_CameraID]->SetCameraTransform(_Transform, _View);
}

void D3DRenderer::LoadModel(const char* _Path, UINT _AssetNum)
{
	std::string spath = _Path;
	std::wstring path;
	path.assign(spath.begin(),spath.end());
	m_pFBXParser->ReadAssetFile(path, _AssetNum);
	static std::function<void(SkeletalNode*)> skeletalFunc = [&](SkeletalNode* _node)
		{
			for (auto& mesh : _node->m_Meshs)
			{
				m_pDevice->CreateVertexBuffer(static_cast<UINT>(mesh.m_Stride * mesh.m_Vertex.size()), mesh.m_Vertex.data(), &mesh.m_pVertexBuffer);
				m_pDevice->CreateIndexBuffer(static_cast<UINT>(mesh.m_IndexCount = mesh.m_Indices.size()), mesh.m_Indices.data(), &mesh.m_pIndexBuffer);
			}
			for (size_t child = 0; child < _node->m_Childs.size(); child++)
			{
				skeletalFunc(&(_node->m_Childs[child]));
			}
		};
	static std::function<void(StaticNode*)> staticFunc = [&](StaticNode* _node)
		{
			for (auto& mesh : _node->m_Meshs)
			{
				m_pDevice->CreateVertexBuffer(static_cast<UINT>(mesh.m_Stride * mesh.m_Vertex.size()), mesh.m_Vertex.data(), &mesh.m_pVertexBuffer);
				m_pDevice->CreateIndexBuffer(static_cast<UINT>(mesh.m_IndexCount = mesh.m_Indices.size()), mesh.m_Indices.data(), &mesh.m_pIndexBuffer);
			}
			for (size_t child = 0; child < _node->m_Childs.size(); child++)
			{
				staticFunc(&(_node->m_Childs[child]));
			}
		};
	// 일단 이렇게 작성한다. 언젠가 더 좋은방법이 떠오르겠지
	if (auto obj = m_pResourceManager->GetAsset<StaticObject>(_AssetNum);
		obj.lock().get() != nullptr)
	{
		staticFunc(obj.lock()->m_pNode.get());
	}
	else if (auto obj = m_pResourceManager->GetAsset<SkeletalObject>(_AssetNum);
		obj.lock().get() != nullptr)
	{
		skeletalFunc(obj.lock()->m_pNode.get());
	}

	// create Material(texture)
	for (auto& material : m_pResourceManager->GetAllResources(typeid(Material).name()))
	{
		auto mat = std::reinterpret_pointer_cast<Material>(material.second);
		if (mat->m_path.empty() == true)
			continue;



		std::string texPath;
		for (size_t texturePathIndex = 0; texturePathIndex < mat->m_path.size(); texturePathIndex++)
		{
			if (mat->m_path[texturePathIndex].empty() == true)
				continue;
			texPath = spath.substr(0, spath.rfind("\\")) + mat->m_path[texturePathIndex];

			// 중복된 텍스쳐리소스가 계속 생성되는것을 방지한다.
			std::weak_ptr<Texture> tx;
			if (m_pResourceManager->GetResource(typeid(Texture).name(), texPath, tx))
			{
				mat->m_pTextures[texturePathIndex] = tx.lock()->m_pTexture;
			}
			else
			{
				m_pDevice->CreateTexture(texPath, &mat->m_pTextures[texturePathIndex]);
				Texture texture;
				texture.m_pTexture = mat->m_pTextures[texturePathIndex];
				m_pResourceManager->AddResource(typeid(Texture).name(), texPath, texture);
			}
		}
		mat->m_path.clear();
		
	}
}

void D3DRenderer::LoadTexture(const char* _Path)
{
	std::string spath = _Path;
	std::wstring path;
	path.assign(spath.begin(), spath.end());

	std::weak_ptr<Texture> tx;
	if (m_pResourceManager->GetResource(typeid(Texture).name(), spath, tx))
	{
		return;
	}

	else
	{
		Texture texture;
		m_pDevice->CreateTexture(path, &texture.m_pTexture);
		m_pResourceManager->AddResource(typeid(Texture).name(), spath, texture);
	}
}

void D3DRenderer::LoadSprite(UINT _AssetNum)
{

	
	std::shared_ptr<SpriteObject> sprite = std::make_shared<SpriteObject>();

	sprite->m_Vertex.resize(4);
	sprite->m_Indices.resize(6);

	sprite->m_Vertex[0].m_UVW = { 0,0,0 };
	sprite->m_Vertex[1].m_UVW = { 1,0,0 };
	sprite->m_Vertex[2].m_UVW = { 0,1,0 };
	sprite->m_Vertex[3].m_UVW = { 1,1,0 };

	sprite->m_Vertex[0].m_Pos = { 0,0,0 };
	sprite->m_Vertex[1].m_Pos = { 1,0,0 };
	sprite->m_Vertex[2].m_Pos = { 0,1,0 };
	sprite->m_Vertex[3].m_Pos = { 1,1,0 };

	sprite->m_Indices[0] = 0;
	sprite->m_Indices[1] = 1;
	sprite->m_Indices[2] = 2;

	sprite->m_Indices[3] = 3;
	sprite->m_Indices[4] = 2;
	sprite->m_Indices[5] = 1;

	m_pDevice->CreateVertexBuffer(static_cast<UINT>(sprite->m_Stride * sprite->m_Vertex.size()),sprite->m_Vertex.data(), &sprite->m_pVertexBuffer,true);
	m_pDevice->CreateIndexBuffer(static_cast<UINT>(sprite->m_IndexCount = sprite->m_Indices.size()), sprite->m_Indices.data(), &sprite->m_pIndexBuffer);

	m_pResourceManager->AddAsset(_AssetNum, sprite);

	

	
}

void D3DRenderer::LoadGameEffect(const char* _Path, UINT _AssetNum)
{
}

void D3DRenderer::AddTextureToSprite(const char* _Path, UINT _AssetNum, int _TextureIndex)
{
	std::weak_ptr<SpriteObject> obj;
	if (obj = m_pResourceManager->GetAsset<SpriteObject>(_AssetNum);
		obj.lock().get() == nullptr)
	{
		MessageBox(NULL, L"Asset Not Loaded", L"AddTextureToSprite Error", NULL);
		return;
	}
	if (obj.lock()->m_pTexture.find(_TextureIndex) != obj.lock()->m_pTexture.end())
	{
		MessageBox(NULL, L"TextureIndex Loaded", L"AddTextureToSprite Error", NULL);
		return;
	}

	std::string spath = _Path;
	std::wstring path;
	path.assign(spath.begin(), spath.end());

	std::weak_ptr<Texture> tx;
	if (m_pResourceManager->GetResource(typeid(Texture).name(), spath, tx))
	{
		obj.lock()->m_pTexture[_TextureIndex] = tx;
		return;
	}

	else
	{
		LoadTexture(_Path);
		if (m_pResourceManager->GetResource(typeid(Texture).name(), spath, tx))
		{
			obj.lock()->m_pTexture[_TextureIndex] = tx;
			return;
		}
		return;

	}

	

}

void D3DRenderer::CreateModel(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, UINT* _State, bool* _IsRender)
{
#ifdef _DEBUG
	//if (_Transform == nullptr || _State == nullptr)
	//{
	//	MessageBox(NULL,L"CreateModel Error",L"CreateModel Error",NULL);
	//}
#endif // _DEBUG
	
	
	if (auto obj = m_pResourceManager->GetAsset<StaticObject>(*_AssetNum);
		obj.lock().get() != nullptr)
	{
		m_pObjectManager->AddRenderingObject<StaticObject>(_AssetNum,_ObjectID,nullptr,_Transform,_State, _IsRender,nullptr);
	}
	else if (auto obj = m_pResourceManager->GetAsset<SkeletalObject>(*_AssetNum);
		obj.lock().get() != nullptr)
	{
		m_pObjectManager->AddRenderingObject<SkeletalObject>(_AssetNum, _ObjectID, nullptr, _Transform, _State, _IsRender, nullptr);
	}
	else
	{
		MessageBox(NULL, L"Asset Not Loaded", L"CreateModel Error", NULL);
	}
}

void D3DRenderer::CreateSprite(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, Vector2* _Size, UINT* _State, bool* _IsRender)
{
#ifdef _DEBUG
	//if (_Transform == nullptr || _State == nullptr)
	//{
	//	MessageBox(NULL, L"CreateSprite Error", L"CreateSprite Error", NULL);
	//}
#endif // _DEBUG

	if (auto obj = m_pResourceManager->GetAsset<SpriteObject>(*_AssetNum);
		obj.lock().get() != nullptr)
	{
		m_pObjectManager->AddRenderingObject<SpriteObject>(_AssetNum, _ObjectID,nullptr, _Transform, _State, _IsRender, _Size);
	}
	else
	{
		MessageBox(NULL, L"Asset Not Loaded", L"CreateSprite Error", NULL);
	}
}

void D3DRenderer::CreateEffect(UINT* _AssetNum, UINT _ObjectID, RenderTransform* _Transform, UINT* _State, bool* _IsRender)
{
#ifdef _DEBUG
	//if (_Transform == nullptr || _State == nullptr)
	//{
	//	MessageBox(NULL, L"CreateEffect Error", L"CreateEffect Error", NULL);
	//}
#endif // _DEBUG
}

void D3DRenderer::CreateModel(UINT* _AssetNum, UINT _ObjectID, void* _Transform, UINT* _State, bool* _IsRender)
{
	if (auto obj = m_pResourceManager->GetAsset<StaticObject>(*_AssetNum);
		obj.lock().get() != nullptr)
	{
		m_pObjectManager->AddRenderingObject<StaticObject>(_AssetNum, _ObjectID, _Transform, nullptr, _State, _IsRender, nullptr);
	}
	else if (auto obj = m_pResourceManager->GetAsset<SkeletalObject>(*_AssetNum);
		obj.lock().get() != nullptr)
	{
		m_pObjectManager->AddRenderingObject<SkeletalObject>(_AssetNum, _ObjectID, _Transform, nullptr, _State, _IsRender, nullptr);
	}
	else
	{
		MessageBox(NULL, L"Asset Not Loaded", L"CreateModel Error", NULL);
	}
}

void D3DRenderer::CreateSprite(UINT* _AssetNum, UINT _ObjectID, void* _Transform, Vector2* _Size, UINT* _State, bool* _IsRender)
{
	if (auto obj = m_pResourceManager->GetAsset<SpriteObject>(*_AssetNum);
		obj.lock().get() != nullptr)
	{
		m_pObjectManager->AddRenderingObject<SpriteObject>(_AssetNum, _ObjectID, _Transform, nullptr, _State, _IsRender, _Size);
	}
	else
	{
		MessageBox(NULL, L"Asset Not Loaded", L"CreateSprite Error", NULL);
	}
}

void D3DRenderer::CreateEffect(UINT* _AssetNum, UINT _ObjectID, void* _Transform, UINT* _State, bool* _IsRender)
{
}

void D3DRenderer::DestroyObject(UINT _ObjectID)
{
}

void D3DRenderer::DestroyAllObject()
{
	m_pObjectManager->DestroyAllObjectInAsset<SkeletalObject>();
	m_pObjectManager->DestroyAllObjectInAsset<StaticObject>();
	m_pObjectManager->DestroyAllObjectInAsset<SpriteObject>();

}


void D3DRenderer::DrawDebugingBoundingBox(Vector3 _Center, Vector3 _Extents, Vector4 _Orientation, Vector4 _Color)
{
	static DirectX::BoundingOrientedBox box;
	box.Center = { _Center.x,_Center.y,_Center.z };
	memcpy(&box.Extents, &_Extents, sizeof(Vector3));
	memcpy(&box.Orientation, &_Orientation, sizeof(Vector4));
	static SimpleMath::Vector4 color;
	memcpy(&color, &_Color, sizeof(Vector4));

	m_DebugBox.emplace_back( box ,color);
}

void D3DRenderer::DrawDebugingBoundingSphere(Vector3 _Center, float _Radius, Vector4 _Color)
{
	static DirectX::BoundingSphere sphere;
	sphere.Center = { _Center.x,_Center.y,_Center.z };
	sphere.Radius = _Radius;
	static SimpleMath::Vector4 color;
	memcpy(&color, &_Color, sizeof(Vector4));
	m_DebugSphere.emplace_back(sphere,color);
}

void D3DRenderer::DrawDebugingRay(Vector4 _Origin, Vector4 _Direction, Vector4 _Color, bool _IsNormal)
{
	static SimpleMath::Vector4 origin;
	memcpy(&origin, &_Origin, sizeof(Vector4));

	static SimpleMath::Vector4 direction;
	memcpy(&direction, &_Direction, sizeof(Vector4));

	static SimpleMath::Vector4 color;
	memcpy(&color, &_Color, sizeof(Vector4));

	m_DebugRay.emplace_back(origin, direction, color,_IsNormal);
}

void D3DRenderer::PlayAnimation(UINT _ObjectID)
{
	ObjectInfo* info = FindObject(_ObjectID);
	info->m_IsAnimationPlay = true;
	info->m_IsAnimationComplete = false;
	info->m_CurrentTime = 0;
	info->m_CurrentFrame = 0;

}

void D3DRenderer::StopAnimation(UINT _ObjectID)
{
	FindObject(_ObjectID)->m_IsAnimationPlay = false;
}

void D3DRenderer::SetAnimationTime(UINT _ObjectID, double _time)
{
	FindObject(_ObjectID)->m_CurrentTime = _time;
}

void D3DRenderer::SetAnimationSpeed(UINT _ObjectID, float _Speed)
{
	FindObject(_ObjectID)->m_AnimationSpeed = _Speed;
}

void D3DRenderer::SetAnimationRepeat(UINT _ObjectID, bool _Value)
{
	FindObject(_ObjectID)->m_IsAnimationRefeat = _Value;
}

bool D3DRenderer::GetAnimationComplete(UINT _ObjectID)
{
	return FindObject(_ObjectID)->m_IsAnimationComplete;
}

double D3DRenderer::GetCurrentAnimationTime(UINT _ObjectID)
{
	return FindObject(_ObjectID)->m_CurrentTime;
}

AnimationInfo D3DRenderer::GetAnimationInfo(UINT _AssetNum)
{
	std::weak_ptr<Anim::AnimationInfo> anim;
	AnimationInfo animInfo;
	if (m_pResourceManager->GetResource(typeid(Anim::AnimationInfo).name(), std::to_string(_AssetNum), anim) == true)
	{
		animInfo.AnimationNum = anim.lock()->m_Animation.size();
		for (size_t i = 0; i < animInfo.AnimationNum; i++)
		{
			animInfo.Animation[i].AnimationName = anim.lock()->m_Animation[i].m_Name.c_str();
			animInfo.Animation[i].Duration = anim.lock()->m_Animation[i].m_Duration;
			animInfo.Animation[i].AnimationFullTime = anim.lock()->m_Animation[i].m_AnimationFullTime;
			animInfo.Animation[i].TickPerFrame = anim.lock()->m_Animation[i].m_TickPerFrame;
		}
	}

	return animInfo;
}

ObjectInfo* D3DRenderer::FindObject(UINT _ObjectID)
{
	ObjectInfo* result;
	result = m_pObjectManager->GetObjectInfo<SkeletalObject>(_ObjectID);
	if (result == nullptr)
	{
		result = m_pObjectManager->GetObjectInfo<StaticObject>(_ObjectID);
	}
	if (result == nullptr)
	{
		result = m_pObjectManager->GetObjectInfo<SpriteObject>(_ObjectID);
	}
	if (result == nullptr)
	{
		assert(result != nullptr);
	}
	return result;
}

//void D3DRenderer::FastRender()
//{
//	//m_IsFastRender = true;
//	//m_IsLateRender = false;
//	//DrawStatic();
//	//DrawSkeletal();
//}

void D3DRenderer::NormalRender()
{
	//m_IsFastRender = false;
	m_IsLateRender = false;
	DrawStatic();
	DrawSkeletal();
}

void D3DRenderer::LateRender()
{
	m_IsLateRender = true;
	m_pDevice->ClearDepthView();
	DrawStatic();
	DrawSkeletal();
}

void D3DRenderer::DrawStatic()
{
	m_pDevice->VSSetStatic();
	m_pDevice->PSSetModel();
	if (m_IsFastRender == true)
	{
		m_pDevice->PSSetShadow();
	}
	std::function<void(StaticNode*, SimpleMath::Matrix&)> func = [&](StaticNode* _Node, SimpleMath::Matrix& _Parent) {
		_Node->m_WorldMatrix = _Node->m_LocalMatrix * _Parent;
		for (auto& mesh : _Node->m_Meshs)
		{
			for (auto& material : m_pResourceManager->GetAllResources(typeid(Material).name()))
			{
				auto mat = std::reinterpret_pointer_cast<Material>(material.second);
				if (mesh.m_MaterialIndex == mat->m_Index)
				{
					{
						for (size_t textureIndex = 0; textureIndex < mat->m_pTextures.size(); textureIndex++)
						{
							m_pDevice->SetTexture(textureIndex, 1, mat->m_pTextures[textureIndex], Shader_Type::PixelShader);
						}
					}
					break;
				}
			}
			m_pDevice->SetVertexBuffer(mesh.m_pVertexBuffer, mesh.m_Stride, mesh.m_Offset);
			m_pDevice->SetIndexBuffer(mesh.m_pIndexBuffer, mesh.m_IndexCount);

			m_ObjectBuffer.WorldTransform = (_Node->m_WorldMatrix).Transpose();



			m_pDevice->SetConstantBuffer(CB_Type::ObjectBuffer, &m_ObjectBuffer, Shader_Type::All);
			m_pDevice->RenderPipeline();
			m_pDevice->Draw();

		}
		for (auto& node : _Node->m_Childs)
		{
			func(&node, _Node->m_WorldMatrix);
		}
		};

	

	for (auto& asset : m_pResourceManager->GetAllAssets<StaticObject>())
	{
		for (auto& obj : m_pObjectManager->GetObjectList<StaticObject>()[asset.first])
		{
			if (*obj.m_pIsRender == false)
			{
				continue;
			}
			if (m_IsLateRender == true)
			{
				if (obj.m_IsLateRender == false)
				{
					continue;
				}
				if (obj.m_pWorldTransform == nullptr)
				{
					memcpy(&m_Pos, &obj.m_pWorldTM->Position, sizeof(SimpleMath::Vector3));
					memcpy(&m_Rot, &obj.m_pWorldTM->Rotation, sizeof(SimpleMath::Vector3));
					memcpy(&m_Scale, &obj.m_pWorldTM->Scale, sizeof(SimpleMath::Vector3));
					m_World = (SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateFromYawPitchRoll(m_Rot) * SimpleMath::Matrix::CreateTranslation(m_Pos));
				}
				else
				{
					m_World = *obj.m_pWorldTransform;
				}

				m_ObjectBuffer.ObjectColor = obj.m_ColorPower;

				m_ObjectBuffer.ObjectBaseColor = obj.m_BaseColor;
				m_ObjectBuffer.Metallic = obj.m_Metallic;
				m_ObjectBuffer.Loughness = obj.m_Loughness;



				asset.second->m_pNode->m_WorldMatrix = asset.second->m_pNode->m_LocalMatrix * m_World;

				for (auto& node : asset.second->m_pNode->m_Childs)
				{
					func(&node, asset.second->m_pNode->m_WorldMatrix);
				}
				continue;
			}
			if (obj.m_pWorldTransform == nullptr)
			{
				memcpy(&m_Pos, &obj.m_pWorldTM->Position, sizeof(SimpleMath::Vector3));
				memcpy(&m_Rot, &obj.m_pWorldTM->Rotation, sizeof(SimpleMath::Vector3));
				memcpy(&m_Scale, &obj.m_pWorldTM->Scale, sizeof(SimpleMath::Vector3));
				m_World = (SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateFromYawPitchRoll(m_Rot) * SimpleMath::Matrix::CreateTranslation(m_Pos));
			}
			else
			{
				m_World = *obj.m_pWorldTransform;
			}

			m_ObjectBuffer.ObjectColor = obj.m_ColorPower;

			m_ObjectBuffer.ObjectBaseColor = obj.m_BaseColor;
			m_ObjectBuffer.Metallic = obj.m_Metallic;
			m_ObjectBuffer.Loughness = obj.m_Loughness;



			asset.second->m_pNode->m_WorldMatrix = asset.second->m_pNode->m_LocalMatrix * m_World;

			for (auto& node : asset.second->m_pNode->m_Childs)
			{
				func(&node, asset.second->m_pNode->m_WorldMatrix);
			}

		}


	}
}

void D3DRenderer::DrawSkeletal()
{
	m_pDevice->VSSetSkeletal();
	m_pDevice->PSSetModel();

	if (m_IsFastRender == true)
	{
		m_pDevice->PSSetShadow();
	}
	// 본의 월드 매트릭스를 구한다.
	static std::function<void(SkeletalNode*, std::array<SimpleMath::Matrix, 10>&, UINT, const double&, Anim::AnimationInfo*, UINT, bool)> SetBoneWolrd = [&](SkeletalNode* _Node, std::array<SimpleMath::Matrix, 10>& _Parent, UINT _State, const double& _Time, Anim::AnimationInfo* _Info, UINT _Frame, bool _IsLastFrame)
		{
			{

				for (size_t meshIter = 0; meshIter < _Info->m_Animation[_State].m_Mesh.size(); meshIter++)
				{
					// 본에 해당하지 않는다
					if (_Node->m_IsBone == false || _Node->m_BoneIndex[meshIter] == 0)
					{
						_Node->m_WorldMatrix[meshIter] = _Node->m_LocalMatrix * _Parent[meshIter];
						continue;
					}
					// 본에 해당하는데 애니메이션이 없다. || 본은 있으나 아예 애니메이션이 없다.
					if ((_Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Frame.empty()) == true)
					{
						_Node->m_WorldMatrix[meshIter] = _Node->m_LocalMatrix * _Parent[meshIter];
						m_BoneBuffer[meshIter].BoneTransform[_Node->m_BoneIndex[meshIter] - 1] = (_Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_OffsetMatrix * _Node->m_WorldMatrix[meshIter]).Transpose();
					}
					else
					{
						// 프레임보간.
						SimpleMath::Matrix lerpMatrix;
						{

							UINT frameNum = _Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Time.size();

							for (size_t frameIter = 0; frameIter < _Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Time.size(); frameIter++)
							{
								double time = _Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Time[frameIter];


								if (_Time < time)
								{
									break;
								}
								_Node->m_CurrentFrame = frameIter;
							}

							_Node->m_NextFrame = (_Node->m_CurrentFrame + 1) % frameNum;

							double currTime = _Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Time[(_Node->m_CurrentFrame + _Frame) % frameNum];
							double nextTime = _Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Time[(_Node->m_NextFrame + _Frame) % frameNum];

							if (nextTime <= 0.000001) nextTime = 0.000001;

							
							lerpMatrix = SimpleMath::Matrix::Lerp
							(_Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Frame[_Node->m_CurrentFrame],
								_Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Frame[_Node->m_NextFrame],
								(_Time - currTime) / nextTime);
							if (_IsLastFrame == true)
							{
								lerpMatrix = _Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_Frame.back();
							}
						}
						_Node->m_WorldMatrix[meshIter] = (lerpMatrix * _Parent[meshIter]);
						m_BoneBuffer[meshIter].BoneTransform[_Node->m_BoneIndex[meshIter] - 1] = (_Info->m_Animation[_State].m_Mesh[meshIter].m_Bone[_Node->m_BoneIndex[meshIter] - 1].m_OffsetMatrix * _Node->m_WorldMatrix[meshIter]).Transpose();
					}
				}

			}

			for (size_t chileSize = 0; chileSize < _Node->m_Childs.size(); chileSize++)
			{
				// 재귀를 통해 자식트랜스폼을 월드로 만든다.
				SetBoneWolrd(&_Node->m_Childs[chileSize], _Node->m_WorldMatrix, _State, _Time, _Info, _Frame, _IsLastFrame);
			}
		};

	// 메쉬를 그린다.
	static std::function<void(SkeletalNode*,UINT&)> func = [&](SkeletalNode* _Node, UINT& _State) {

		// 메쉬가 있다.
		if (_Node->m_MeshIndex != 0)
		{
			m_pDevice->SetConstantBuffer(CB_Type::BoneBuffer, &m_BoneBuffer[_Node->m_MeshIndex - 1], Shader_Type::VertexShader);
			for (auto& mesh : _Node->m_Meshs)
			{
				for (auto& material : m_pResourceManager->GetAllResources(typeid(Material).name()))
				{
					auto mat = std::reinterpret_pointer_cast<Material>(material.second);
					if (mesh.m_MaterialIndex == mat->m_Index)
					{
						{
							for (size_t textureIndex = 0; textureIndex < mat->m_pTextures.size(); textureIndex++)
							{
								m_pDevice->SetTexture(textureIndex, 1, mat->m_pTextures[textureIndex], Shader_Type::PixelShader);
							}
						}
						break;
					}
				}
				m_pDevice->SetVertexBuffer(mesh.m_pVertexBuffer, mesh.m_Stride, mesh.m_Offset);
				m_pDevice->SetIndexBuffer(mesh.m_pIndexBuffer, mesh.m_IndexCount);




				m_pDevice->RenderPipeline();
				m_pDevice->Draw();

			}
		}
		for (size_t node = 0; node < _Node->m_Childs.size(); node++)
		{
			func(&_Node->m_Childs[node], _State);
		}
		};




	

	// 오브젝트들을 일단 다 그린다.
	for (auto& asset : m_pResourceManager->GetAllAssets<SkeletalObject>())
	{

		
		// 오브젝트들을 일단 다 그린다.
		{
			for (auto& bone : m_BoneBuffer)
			{
				for (auto& matrix : bone.BoneTransform)
				{
					matrix = SimpleMath::Matrix::Identity;
				}
			}
		}

		for (auto& obj : m_pObjectManager->GetObjectList<SkeletalObject>()[asset.first])
		{
			
			if (*obj.m_pIsRender == false)
			{
				continue;
			}
			if (m_IsLateRender == true)
			{
				if (obj.m_IsLateRender == false)
				{
					continue;
				}
			}
			m_ObjectBuffer.WorldTransform = SimpleMath::Matrix::Identity;

			if (obj.m_pWorldTransform == nullptr)
			{
				memcpy(&m_Pos, &obj.m_pWorldTM->Position, sizeof(SimpleMath::Vector3));
				memcpy(&m_Rot, &obj.m_pWorldTM->Rotation, sizeof(SimpleMath::Vector3));
				memcpy(&m_Scale, &obj.m_pWorldTM->Scale, sizeof(SimpleMath::Vector3));
				m_World = (SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateFromYawPitchRoll(m_Rot) * SimpleMath::Matrix::CreateTranslation(m_Pos));
			}
			else
			{
				m_World = *obj.m_pWorldTransform;
			}
			
			m_ObjectBuffer.ObjectColor = obj.m_ColorPower;

			m_ObjectBuffer.ObjectBaseColor = obj.m_BaseColor;
			m_ObjectBuffer.Metallic = obj.m_Metallic;
			m_ObjectBuffer.Loughness = obj.m_Loughness;
			

			std::array<SimpleMath::Matrix, 10> world;
			world.fill(m_World);

			// 해당 에셋에 애니메이션이 있다면
			std::weak_ptr<Anim::AnimationInfo> info;
			if (
				m_pResourceManager->GetResource(typeid(Anim::AnimationInfo).name(), std::to_string(asset.first), info) == true)
			{
				

				double fullTime = info.lock()->m_Animation[obj.m_CurrentSTATE].m_AnimationFullTime;


				if (obj.m_CurrentTime > fullTime)
				{
					obj.m_IsAnimationComplete = true;
					

					// 하트박스 & 아이템박스일떄 열린채로 있어야하기에.
					if (asset.first == 6 || asset.first == 7 || obj.m_ForBox == true)
					{
						obj.m_ForBox = true;
						SetBoneWolrd(asset.second->m_pNode.get(), world, obj.m_CurrentSTATE, 0, info.lock().get(), 0, true);
					}
					else
					{
						if (obj.m_IsAnimationRefeat == true)
						{

							obj.m_CurrentTime = obj.m_CurrentTime - fullTime;

							double frameTime = info.lock()->m_Animation[obj.m_CurrentSTATE].m_TickPerFrame;
							while (obj.m_CurrentTime > frameTime)
							{
								obj.m_CurrentTime -= frameTime;
							}

							SetBoneWolrd(asset.second->m_pNode.get(), world, obj.m_CurrentSTATE, obj.m_CurrentTime, info.lock().get(), 1, false);
						}
						else
						{
							//fullTime -= info.lock()->m_Animation[obj.m_CurrentSTATE].m_TickPerFrame;
							SetBoneWolrd(asset.second->m_pNode.get(), world, obj.m_CurrentSTATE, fullTime, info.lock().get(), 1, true);
						}
					}

				}
				else
				{
					if (obj.m_IsAnimationPlay == true)
					{
						obj.m_IsAnimationComplete = false;
					}
					SetBoneWolrd(asset.second->m_pNode.get(), world, obj.m_CurrentSTATE, obj.m_CurrentTime, info.lock().get(),0,false);
				}
			}
			else
			{
				//SetBoneWolrd(asset.second->m_pNode.get(), world, obj.m_CurrentSTATE, obj.m_CurrentTime, nullptr, 0);
			}
			
			m_pDevice->SetConstantBuffer(CB_Type::ObjectBuffer, &m_ObjectBuffer, Shader_Type::All);

			for (size_t node = 0; node < asset.second->m_pNode->m_Childs.size(); node++)
			{
				func(&asset.second->m_pNode->m_Childs[node], obj.m_CurrentSTATE);
			}

		}


	}
}

void D3DRenderer::AnimationUpdate(double _DeltaTime)
{
	// 해당 에셋을 공유하는
	for (auto& asset : m_pResourceManager->GetAllAssets<SkeletalObject>())
	{

		for (auto& obj : m_pObjectManager->GetObjectList<SkeletalObject>()[asset.first])
		{

			{
				if (obj.m_pSTATE == nullptr)
					obj.m_CurrentSTATE = 0;
				else
				{
					if (obj.m_CurrentSTATE != *obj.m_pSTATE)
					{
						obj.m_CurrentSTATE = *obj.m_pSTATE;

						obj.m_CurrentTime = 0;
						// 스테이트가 바뀌면 프레임을 0으로 되돌린다.
						obj.m_CurrentFrame = 0;
					}
					
				}
				if (obj.m_IsAnimationPlay == true)
				{
					obj.m_CurrentTime += _DeltaTime * obj.m_AnimationSpeed;
				}
			}
		}
	}
}

// todo : 나중에 프레임 끝난 시점도 반환하면 좋겠다.

void D3DRenderer::DrawSprite()
{
	
	m_pDevice->VSSetSprite();
	m_pDevice->PSSetSprite();
	for (auto& asset : m_pResourceManager->GetAllAssets<SpriteObject>())
	{
		for (auto& obj : m_pObjectManager->GetObjectList<SpriteObject>()[asset.first])
		{
			if (*obj.m_pIsRender == false)
				continue;

			if (obj.m_pWorldTransform == nullptr)
			{
				memcpy(&m_Pos, &obj.m_pWorldTM->Position, sizeof(SimpleMath::Vector3));
				memcpy(&m_Rot, &obj.m_pWorldTM->Rotation, sizeof(SimpleMath::Vector3));
				memcpy(&m_Scale, &obj.m_pWorldTM->Scale, sizeof(SimpleMath::Vector3));


				     

				m_World = (SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateFromYawPitchRoll(m_Rot) * SimpleMath::Matrix::CreateTranslation(m_Pos));
			}
			else
			{
				m_World = *obj.m_pWorldTransform;
			}
			if (obj.m_pSize != nullptr)
			{
				if ((asset.second->m_Size.x != obj.m_pSize->x) || (asset.second->m_Size.y != obj.m_pSize->y))
				{
					
					
				}

				asset.second->m_Size.x = obj.m_pSize->x * 2;
				asset.second->m_Size.y = obj.m_pSize->y * 2;
				float x, y, z, width, height;

				width = asset.second->m_Size.x;
				height = asset.second->m_Size.y;

				m_World._42 = -m_World._42;

				x = m_World._41;
				y = m_World._42;
				z = m_World._43;


				// 일단 좌상단 기준으로 만든다.
				if (obj.m_IsOriginCenter == false)
				{
					asset.second->m_Vertex[0].m_Pos = { x,y,z };
					asset.second->m_Vertex[1].m_Pos = { x + width,y,z };
					asset.second->m_Vertex[2].m_Pos = { x,(y) - height,z};
					asset.second->m_Vertex[3].m_Pos = { x + width,(y) - height,z};
				}
				
				D3D11_MAPPED_SUBRESOURCE sub{};
				m_pDevice->GetDeviceContext()->Map(asset.second->m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);

				for (size_t i = 0; i < asset.second->m_Vertex.size(); i++)
				{
					static_cast<StaticVertex*>(sub.pData)[i] = asset.second->m_Vertex[i];
				}


				m_pDevice->GetDeviceContext()->Unmap(asset.second->m_pVertexBuffer.Get(), 0);
			}


			m_pDevice->SetVertexBuffer(asset.second->m_pVertexBuffer, asset.second->m_Stride, asset.second->m_Offset);
			m_pDevice->SetIndexBuffer(asset.second->m_pIndexBuffer, asset.second->m_IndexCount);

			m_ObjectBuffer.WorldTransform = (m_World).Transpose();
			m_pDevice->SetConstantBuffer(CB_Type::ObjectBuffer, &m_ObjectBuffer, Shader_Type::All);
			m_pDevice->SetTexture(0, 1, asset.second->m_pTexture[*obj.m_pSTATE].lock()->m_pTexture, Shader_Type::PixelShader);
			

			m_pDevice->RenderPipeline();
			m_pDevice->Draw();
		}
	}
}

float D3DRenderer::GetAspectRatio() const
{
	
	return static_cast<float>(m_ScreenWidth) / m_ScreenHeight;
}

void D3DRenderer::Finalize()
{
#ifdef _DEBUG
	DebugDraw::Uninitialize();
#endif // _DEBUG

}