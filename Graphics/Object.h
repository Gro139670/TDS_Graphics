#pragma once
#include "ConstantBuffer.h"
#include <array>

class SkeletalObject;
class StaticObject;
class SpriteObject;


#pragma region Data
struct Material
{
	std::vector<std::string> m_path;
	UINT m_Index = 0;
	std::vector<ComPtr<ID3D11ShaderResourceView>> m_pTextures;
	float m_Metalic;
	float m_Loughness;
};

struct Texture
{
	ComPtr<ID3D11ShaderResourceView> m_pTexture = nullptr;
	Vector2 m_Size;
};

struct StaticVertex
{
	SimpleMath::Vector3 m_Pos{};		// 버텍스 위치
	SimpleMath::Vector3 m_UVW{};		//
	SimpleMath::Vector3 m_Normal{};	//
	SimpleMath::Vector3 m_Tangent{};	//
};

struct SkeletalVertex : public StaticVertex
{
	float m_BoneWeights[3] = {0,0,0};		// 본의 가중치값
	float m_BoneIndex[4] = {0,0,0,0};		// 본의 인덱스
};

class MeshBase
{
public:
	virtual ~MeshBase() {};
	ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_pIndexBuffer = nullptr;

	// 디버그모드로 인덱스가 잘 들어오는지 확인한다.

	std::vector<UINT> m_Indices;			// 버텍스의 인덱스들을 저장합니다.


	UINT m_IndexCount = 0;
	UINT m_Offset = 0;
	UINT m_MaterialIndex = 0;

};



// 템플릿으로 작성하지 않는다. 
class StaticMesh : public MeshBase
{
public:
	virtual ~StaticMesh() {};

	std::vector<StaticVertex> m_Vertex;

	const static size_t m_Stride = sizeof(StaticVertex);

};

class SpriteObject : public StaticMesh
{
public:
	SimpleMath::Matrix m_WorldMatrix;
	std::map<UINT,std::weak_ptr<Texture>> m_pTexture;
	bool m_IsOriginCenter = false;
	Vector2 m_Size;
};

class SkeletalMesh : public MeshBase
{
public:
	SkeletalMesh() {};
	std::vector<SkeletalVertex> m_Vertex;

	const static size_t m_Stride = sizeof(SkeletalVertex);
};

#pragma endregion
#pragma region Node
/// <summary>
/// 이곳에 파싱할 데이터를 정의 해본다.
/// 처음 생성되는 오브젝트가 루트 노드이다.
/// 원래는 오브젝트로 설정하려 했으나 한번 감쌀 필요성을 느껴 노드라 명명하고 오브젝트로 감싼다.
/// </summary>

class Node
{
public:
	Node() {}
	virtual ~Node() {}
public:
	// 노드안에 들어있는 트랜스폼 정보.
	SimpleMath::Matrix m_LocalMatrix{};

};

class StaticNode : public Node
{
public:
	StaticNode() {}
	~StaticNode() {}

public:
	SimpleMath::Matrix m_WorldMatrix{};

	// 다음 노드들을 이곳에 담는다.
	std::vector<StaticNode> m_Childs;

	// 한 노드에 메쉬가 여러개 들어 있을 수 있다.
	std::vector<StaticMesh> m_Meshs;

	StaticNode* m_pParent;

	bool m_IsHasMesh = false;


};

class SkeletalNode : public Node
{
public:
	SkeletalNode() {}
	~SkeletalNode() {}
public:
	// 매쉬마다 다른 월드 트랜스폼이 필요하다.
// 시간이 없으므로 넉넉하게 100개로 잡는다.
	std::array<SimpleMath::Matrix, 10> m_WorldMatrix;
	// 다음 노드들을 이곳에 담는다.
	std::vector<SkeletalNode> m_Childs;

	// 한 노드에 메쉬가 여러개 들어 있을 수 있다.
	std::vector<SkeletalMesh> m_Meshs;

	std::vector < UINT> m_BoneIndex;
	UINT m_MeshIndex = 0;
	bool m_IsBone = false;

	UINT m_CurrentFrame = 0;
	UINT m_NextFrame = 0;
	SkeletalNode* m_pParent= nullptr;

	// 애니메이션에서 석궁을 위한 노드
	SkeletalNode* m_pWeapon = nullptr;

	SimpleMath::Matrix m_ParentMatrix;
};

#pragma endregion
#pragma region Object

class Object
{
public:
	virtual ~Object() {}

public:
	UINT m_ID = 0;
	UINT m_MeshNum = 0;

	// 이 오브젝트가 불투명한가?
	bool m_IsOpaque = true;
};

class StaticObject : public Object
{
public:
	StaticObject()
	{
		m_pNode = std::make_shared<StaticNode>();
	}
	~StaticObject() {}

public:
	std::shared_ptr<StaticNode> m_pNode;


};





class SkeletalObject : public Object
{
public:
	SkeletalObject()
	{
		m_pNode = std::make_shared<SkeletalNode>();
	}
	~SkeletalObject() {}

public:
	std::shared_ptr<SkeletalNode> m_pNode;


	// 이 오브젝트가 불투명한가?
	bool m_IsOpaque = true;

};

#pragma endregion

namespace Anim
{
	struct Bone
	{
		std::vector<SimpleMath::Matrix> m_Frame;
		// 본에 들어있는 오프셋 정보.
		// 끼얏호우 오프셋도 매쉬마다 다르다.
		SimpleMath::Matrix m_OffsetMatrix;
		std::vector<double> m_Time;

	};

	struct Clip
	{
		// 본 순서대로 넣는다.
		std::vector<Bone> m_Bone;


	};

	struct Animation
	{
		// 애니메이션 이름은?
		std::string m_Name;
		// 어떤 메쉬에 적용될 애니메이션인가?
		std::vector<Clip> m_Mesh;

		// 애니메이션 초
		double m_Duration = 0;
		// 초당 프레임
		double m_TickPerSec = 0;
		// 애니메이션이 재생되는 전체시간
		double m_AnimationFullTime = 0;
		// 1프레임에 걸리는 시간
		double m_TickPerFrame = 0;
	};

	struct AnimationInfo
	{
		// 어떤 모델이 가지고 있는 애니메이션인가>?
		UINT m_AssetNum = 0;
		// 몇번째 상태에 나오는 애니메이션인가>'
		std::vector<Animation> m_Animation;
	};
}