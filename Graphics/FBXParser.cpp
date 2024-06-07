#include "pch.h"
#include "FBXParser.h"
#include <filesystem>
#include <functional>


Converter::Converter()
{
	

	m_pScene = nullptr;
}

Converter::~Converter()
{

}

void Converter::ReadAssetFile(std::wstring file, UINT _AssetNum)
{
	Assimp::Importer m_importer;

	if (m_pResourceManager->GetAsset<SkeletalObject>(_AssetNum).lock().get() != nullptr ||
		m_pResourceManager->GetAsset<StaticObject>(_AssetNum).lock().get() != nullptr
		)
	{
		return;
	}

	m_IsMakeAnimation = false;
	m_NumMesh = 0;

	std::wstring fileStr = file;

	auto p = std::filesystem::path(fileStr);
	assert(std::filesystem::exists(p));

	const aiScene* scene;
	scene = m_importer.ReadFile(
		ToString(fileStr),
		aiProcess_ConvertToLeftHanded |
		aiProcess_Triangulate |
		aiProcess_GenUVCoords |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_PopulateArmatureData |
		aiProcess_LimitBoneWeights |
		aiProcess_GenBoundingBoxes
	);


	assert(scene != nullptr);
	m_pScene = scene;
	bool hasBone = false;
	aiString path;
	for (size_t meshIter = 0; meshIter < m_pScene->mNumMeshes; meshIter++)
	{
		if (m_pScene->mMeshes[meshIter]->HasBones() == true)
		{
			hasBone = true;
			break;
		}
	}

	if (hasBone == true)
	{
		{
			std::shared_ptr<SkeletalObject> asset = std::make_shared<SkeletalObject>();
			ProcessSkeletalNode(m_pScene->mRootNode, asset->m_pNode.get(),AiMatrixToMatrix(m_pScene->mRootNode->mTransformation));
			asset->m_MeshNum = m_pScene->mNumMeshes;


			for (size_t materials = 0; materials < m_pScene->mNumMaterials; materials++)
			{
				// 일단 투명맵텍스쳐로만 판단한다.
				if (AI_SUCCESS == m_pScene->mMaterials[materials]->GetTexture(aiTextureType_OPACITY, 0, &path))
				{
					asset->m_IsOpaque = true;
					break;
				}
			}
			
			m_pResourceManager->AddAsset(_AssetNum, asset);
		}

		
	}
	else
	{
		{
			std::shared_ptr<StaticObject> asset = std::make_shared<StaticObject>();
			ProcessStaticNode(m_pScene->mRootNode, asset->m_pNode.get());
			asset->m_MeshNum = m_pScene->mNumMeshes;

			for (size_t materials = 0; materials < m_pScene->mNumMaterials; materials++)
			{
				if (AI_SUCCESS == m_pScene->mMaterials[materials]->GetTexture(aiTextureType_OPACITY, 0, &path))
				{
					asset->m_IsOpaque = true;
					break;
				}
			}

			m_pResourceManager->AddAsset(_AssetNum, asset);
		}
	}


	{
#pragma region createAnim
		// create Animation
		if (m_pScene->HasAnimations() || hasBone == true)
		{
			Anim::AnimationInfo Anime;
			Anime.m_AssetNum = _AssetNum;
			
			// 애니메이션 수만큼 만든다.
			Anime.m_Animation.resize(m_pScene->mNumAnimations);
			for (size_t animSize = 0; animSize < m_pScene->mNumAnimations; animSize++)
			{
				Anime.m_Animation[animSize].m_Name = m_pScene->mAnimations[animSize]->mName.C_Str();
				Anime.m_Animation[animSize].m_Duration = m_pScene->mAnimations[animSize]->mDuration;
				Anime.m_Animation[animSize].m_TickPerSec = m_pScene->mAnimations[animSize]->mTicksPerSecond;
				Anime.m_Animation[animSize].m_AnimationFullTime = Anime.m_Animation[animSize].m_Duration / Anime.m_Animation[animSize].m_TickPerSec;
				Anime.m_Animation[animSize].m_TickPerFrame = Anime.m_Animation[animSize].m_AnimationFullTime / Anime.m_Animation[animSize].m_Duration;


				// 가지고 있는 메쉬만큼 만든다.
				Anime.m_Animation[animSize].m_Mesh.resize(m_pScene->mNumMeshes);
				for (size_t meshSize = 0; meshSize < m_pScene->mNumMeshes; meshSize++)
				{
					// 메쉬가 가진 본 만큼 만든다.
					Anime.m_Animation[animSize].m_Mesh[meshSize].m_Bone.resize(m_pScene->mMeshes[meshSize]->mNumBones);

					// 현재 본의 노드와
					for (size_t boneSize = 0; boneSize < m_pScene->mMeshes[meshSize]->mNumBones; boneSize++)
					{
						for (size_t channelSize = 0; channelSize < m_pScene->mAnimations[animSize]->mNumChannels; channelSize++)
						{
							// (일단 코드길이좀 줄이고)
							aiString name = m_pScene->mAnimations[animSize]->mChannels[channelSize]->mNodeName;
							// 채널의 노드가 같으면 프레임을 불러온다.
							if (m_pScene->mRootNode->FindNode(name) == m_pScene->mMeshes[meshSize]->mBones[boneSize]->mNode)
							{
								SimpleMath::Vector3 pos, scale;
								SimpleMath::Quaternion quat;
								aiVector3D aiPos, aiScale;
								aiQuaternion aiQuat;
								double time;
								for (size_t keySize = 0; keySize < m_pScene->mAnimations[animSize]->mChannels[channelSize]->mNumPositionKeys; keySize++)
								{
									aiPos = m_pScene->mAnimations[animSize]->mChannels[channelSize]->mPositionKeys[keySize].mValue;
									aiQuat = m_pScene->mAnimations[animSize]->mChannels[channelSize]->mRotationKeys[keySize].mValue;
									aiScale = m_pScene->mAnimations[animSize]->mChannels[channelSize]->mScalingKeys[keySize].mValue;
									time = m_pScene->mAnimations[animSize]->mChannels[channelSize]->mPositionKeys[keySize].mTime;
					/*				memcpy(&pos, &aiPos, sizeof(SimpleMath::Vector3));
									memcpy(&scale, &aiScale, sizeof(SimpleMath::Vector3));*/
									// 심플매스 쿼터니언하고 어심프 쿼터니언이 다르네..? xyzw wxyz
									quat = { aiQuat.x,aiQuat.y,aiQuat.z,aiQuat.w };
									pos = { aiPos.x,aiPos.y,aiPos.z };
									scale = { aiScale.x,aiScale.y,aiScale.z };
									Anime.m_Animation[animSize].m_Mesh[meshSize].m_Bone[boneSize].m_Frame.push_back(SimpleMath::Matrix::CreateScale(scale)* SimpleMath::Matrix::CreateFromQuaternion(quat)* SimpleMath::Matrix::CreateTranslation(pos));


									time *= Anime.m_Animation[animSize].m_TickPerFrame;
									// 모르겠다.,. 8바이트 값이니 그냥 푸시백한다.
									Anime.m_Animation[animSize].m_Mesh[meshSize].m_Bone[boneSize].m_Time.push_back(time);
								}
								// 다불러왔으면 다음 본의 채널찾기를 반복한다.
								break;
							}
						}
						Anime.m_Animation[animSize].m_Mesh[meshSize].m_Bone[boneSize].m_OffsetMatrix = AiMatrixToMatrix(m_pScene->mMeshes[meshSize]->mBones[boneSize]->mOffsetMatrix);
					}
					
				}
			}

			if (Anime.m_Animation.empty() == true)
			{
				Anime.m_Animation.resize(1);
				Anime.m_Animation[0].m_Mesh.resize(m_pScene->mNumMeshes);
				Anime.m_Animation[0].m_Name = "is not Animation. for bone";
				Anime.m_Animation[0].m_AnimationFullTime = D3D11_FLOAT32_MAX;
				Anime.m_Animation[0].m_TickPerFrame = D3D11_FLOAT32_MAX;


				for (size_t meshSize = 0; meshSize < m_pScene->mNumMeshes; meshSize++)
				{
					// 메쉬가 가진 본 만큼 만든다.
					Anime.m_Animation[0].m_Mesh[meshSize].m_Bone.resize(m_pScene->mMeshes[meshSize]->mNumBones);
					for (size_t boneSize = 0; boneSize < m_pScene->mMeshes[meshSize]->mNumBones; boneSize++)
					{
						Anime.m_Animation[0].m_Mesh[meshSize].m_Bone[boneSize].m_OffsetMatrix = AiMatrixToMatrix(m_pScene->mMeshes[meshSize]->mBones[boneSize]->mOffsetMatrix);
					}
				}

			}
			// 애니메이션이 다만들어졌으면 리소스매니저에 일단 넣어준다.
			m_pResourceManager->AddResource(typeid(Anim::AnimationInfo).name(), std::to_string(_AssetNum), Anime);
			// 테스트 결과 일단 잘된다.
			// 렌더러에서 불러와 보자.
		}
#pragma endregion

		for (size_t materials = 0; materials < m_pScene->mNumMaterials; materials++)
		{
			Material mat;
			for (size_t textures = 0; textures < AI_TEXTURE_TYPE_MAX; textures++)
			{
				// 무조건 텍스쳐는 맵마다 1장만 들어있다고 가정한다.
				if (AI_SUCCESS == m_pScene->mMaterials[materials]->GetTexture((aiTextureType)textures, 0, &path))
				{
					mat.m_path.push_back(path.C_Str());
					mat.m_path.back() = mat.m_path.back().substr(mat.m_path.back().rfind("\\"));
				}
				else
				{
					mat.m_path.emplace_back("");
				}
				
			}
			mat.m_pTextures.resize(mat.m_path.size());
			mat.m_Index = m_MaterialID;
			m_MaterialID++;
			m_pResourceManager->AddResource(typeid(Material).name(), m_pScene->mMaterials[materials]->GetName().C_Str() + std::to_string(_AssetNum), mat);
		}
		
	}

	
}

// aiNode의 정보를 StaticNode의 정보에 넣어주는 과정
void Converter::ProcessStaticNode(aiNode* node, StaticNode* _Node)
{


	// staticNode가 가지고 있는 정보
	// 노드안에 들어있는 트랜스폼 정보.
	_Node->m_LocalMatrix = AiMatrixToMatrix(node->mTransformation);
	// aiNode의 Mesh에 정보가 있으니까
	// 정보를 다 빼낸 다음에 변환을 해야한다.

	for (size_t meshs = 0; meshs < node->mNumMeshes; meshs++)
	{
		_Node->m_Meshs.push_back(ReadStaticMeshData(m_pScene->mMeshes[node->mMeshes[meshs]]));

		// 일단 메쉬를 리소스로 넣는것은 나중에 하자.
		//m_pResourceManager->AddResource(typeid(_Node.m_Meshs.back()).name(),, _Node.m_Meshs.back());
	}

	// sNode의 역참조를 스테틱 노드에 넣어준다


	// 다음 노드들을 이곳에 담는다.
	for (int i = 0; i < node->mNumChildren; i++)
	{
		if (node->mNumChildren > 0)
		{
			_Node->m_Childs.emplace_back();
			_Node->m_Childs.back().m_pParent = _Node;
			ProcessStaticNode(node->mChildren[i], &(_Node->m_Childs.back()));
		}
	}
}

void Converter::ProcessSkeletalNode(aiNode* node, SkeletalNode* _Node,const SimpleMath::Matrix& _Parent)
{

	_Node->m_LocalMatrix = AiMatrixToMatrix(node->mTransformation);
	if (node->mNumMeshes > 0)
	{
		if (_Node->m_pParent != nullptr)
		{
			if (_Node->m_pParent->m_IsBone == true)
			{
				_Node->m_ParentMatrix = AiMatrixToMatrix(node->mParent->mTransformation);
				_Node->m_BoneIndex = _Node->m_pParent->m_BoneIndex;
			}
		}
		_Node->m_MeshIndex = m_NumMesh + 1;

	}

	for (size_t meshs = 0; meshs < node->mNumMeshes; meshs++)
	{
		_Node->m_Meshs.push_back(ReadSkeletalMeshData(m_pScene->mMeshes[node->mMeshes[meshs]]));
		m_NumMesh++;


		// 일단 메쉬를 리소스로 넣는것은 나중에 하자.
		//m_pResourceManager->AddResource(typeid(_Node.m_Meshs.back()).name(),, _Node.m_Meshs.back());
	}

	// 비효율적인 코드로 보인다. 고칠 수 있을거 같다. 하지만 시간문제상 이렇게 한다.
	if (m_pScene->HasAnimations())
	{
		//if (_Node->m_IsBone == false)
		{
			UINT bonenum = 0;
			_Node->m_BoneIndex.resize(m_pScene->mNumMeshes);
			for (size_t meshSize = 0; meshSize < m_pScene->mNumMeshes; meshSize++)
			{
				for (size_t bones = 0; bones < m_pScene->mMeshes[meshSize]->mNumBones; bones++)
				{
					if (m_pScene->mMeshes[meshSize]->mBones[bones]->mNode == node)
					{
						_Node->m_BoneIndex[meshSize] = (bones + 1);
						_Node->m_IsBone = true;
						break;
					}
				}
			}
		}
	}
	for (int i = 0; i < node->mNumChildren; i++)
	{
		if (node->mNumChildren > 0)
		{
			_Node->m_Childs.push_back(SkeletalNode());
			_Node->m_Childs.back().m_pParent = _Node;
			ProcessSkeletalNode(node->mChildren[i], &_Node->m_Childs.back(), _Node->m_LocalMatrix);
		}
	}
}


std::string Converter::ToString(const std::wstring& wstr)
{
	return std::string().assign(wstr.begin(), wstr.end());
}

SimpleMath::Matrix Converter::AiMatrixToMatrix(const aiMatrix4x4 aiMatrix)
{
	SimpleMath::Matrix result2;

	memcpy(&result2, &aiMatrix,sizeof(SimpleMath::Matrix));

	return result2.Transpose();
}



StaticMesh Converter::ReadStaticMeshData(aiMesh* mesh)
{

	StaticMesh smesh;
	smesh.m_MaterialIndex = mesh->mMaterialIndex + m_MaterialID;
		// 값을 복사하는 과정
		for (int v = 0; v < mesh->mNumVertices; v++)
		{
			// 버텍스
			StaticVertex vertex;
			memcpy(&vertex.m_Pos, &mesh->mVertices[v], sizeof(SimpleMath::Vector3));

			// UV
			if (mesh->HasTextureCoords(0))
				memcpy(&vertex.m_UVW, &mesh->mTextureCoords[0][v], sizeof(SimpleMath::Vector3));

			// 노말
			if (mesh->HasNormals())
				memcpy(&vertex.m_Normal, &mesh->mNormals[v], sizeof(SimpleMath::Vector3));

			// 탄젠트
			if (mesh->HasTangentsAndBitangents())
				memcpy(&vertex.m_Tangent, &mesh->mTangents[v], sizeof(SimpleMath::Vector3));

			smesh.m_Vertex.push_back(vertex);
		}

		// 인덱스
		for (int j = 0; j < mesh->mNumFaces; j++)
		{
			aiFace& face = mesh->mFaces[j];

			for (int k = 0; k < face.mNumIndices; k++)
			{
				smesh.m_Indices.push_back(face.mIndices[k]);
			}
		}

		return smesh;
	
}

SkeletalMesh Converter::ReadSkeletalMeshData(aiMesh* mesh)
{
	SkeletalMesh smesh;
	smesh.m_MaterialIndex = mesh->mMaterialIndex + m_MaterialID;
	// 값을 복사하는 과정
	for (int v = 0; v < mesh->mNumVertices; v++)
	{
		// 버텍스
		SkeletalVertex vertex;
		memcpy(&vertex.m_Pos, &mesh->mVertices[v], sizeof(SimpleMath::Vector3));

		// UV
		if (mesh->HasTextureCoords(0))
			memcpy(&vertex.m_UVW, &mesh->mTextureCoords[0][v], sizeof(SimpleMath::Vector2));

		// 노말
		if (mesh->HasNormals())
			memcpy(&vertex.m_Normal, &mesh->mNormals[v], sizeof(SimpleMath::Vector3));

		// 탄젠트
		if (mesh->HasTangentsAndBitangents())
			memcpy(&vertex.m_Tangent, &mesh->mTangents[v], sizeof(SimpleMath::Vector3));

		smesh.m_Vertex.push_back(vertex);
	}

	// 인덱스
	for (int j = 0; j < mesh->mNumFaces; j++)
	{
		aiFace& face = mesh->mFaces[j];

		for (int k = 0; k < face.mNumIndices; k++)
		{
			smesh.m_Indices.push_back(face.mIndices[k]);
		}
	}




	for (size_t bones = 0; bones < mesh->mNumBones; bones++)
	{
		UINT vID;
		for (size_t whight = 0; whight < mesh->mBones[bones]->mNumWeights; whight++)
		{
			vID = mesh->mBones[bones]->mWeights[whight].mVertexId;
			for (size_t i = 0; i < 3; i++)
			{
				if (smesh.m_Vertex[vID].m_BoneWeights[i] <= 0)
				{
					smesh.m_Vertex[vID].m_BoneIndex[i] = bones;
					smesh.m_Vertex[vID].m_BoneWeights[i] = mesh->mBones[bones]->mWeights[whight].mWeight;
					break;
				}
			}
		}
	}

	// 스테틱 매쉬의 벡터에 포인터를 넣어준다
	return smesh;
}