#pragma once

#include "ObjectManager.h"
#include "ResourceManager.h"


#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>

#ifdef _DEBUG
#pragma comment(lib, "assimp-vc143-mtd.lib")
#pragma comment(lib, "tinyxml2d.lib")
#else
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif


/// <summary>
/// assimp를 사용한 fbx parser의 프로토타입
/// 동휘가 만들다 용준에게 바톤 넘김
/// 
/// 기본적으로 Converter를 만들고
/// ReadFile() 후 GetStaticRootNode()하면
/// fbx의 첫번째 루트 노드가 반환된다.
/// 메쉬 정보는 스테틱 노드의 벡터 내부의 스테틱 노드에 저장됨
/// 
/// 
/// 2024. 02. 06
/// </summary>
class Converter
{
public:
	Converter();
	~Converter();

public:
	void ReadAssetFile(std::wstring file,UINT _AssetNum);
	void SetResourceManager(std::shared_ptr<Render::ResourceManager> _Value) { m_pResourceManager = _Value; };

private:
	std::string ToString(const std::wstring& wstr);
	SimpleMath::Matrix AiMatrixToMatrix(const aiMatrix4x4 aiMatrix);


	void ProcessStaticNode(aiNode* node, StaticNode* _Node);
	void ProcessSkeletalNode(aiNode* node, SkeletalNode* _Node,const SimpleMath::Matrix& _Parent);
	StaticMesh ReadStaticMeshData(aiMesh* mesh);
	SkeletalMesh ReadSkeletalMeshData(aiMesh* mesh);


private:
	std::shared_ptr<Render::ResourceManager> m_pResourceManager;

private:

private:
	const aiScene* m_pScene;

	bool m_IsMakeAnimation = false;

private:
	UINT m_MaterialID = 0;
	UINT m_NumMesh = 0;
};
