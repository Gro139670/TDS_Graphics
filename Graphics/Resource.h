#pragma once

class ResourceManager;


/// <summary>
/// FBX정보들을 가져와서 버퍼, 텍스쳐, 쉐이더를 만들고 넘겨줄 예정
/// 리소스들의 인터페이스와 같은 역할을 함 IResource로 바꿔야되나
/// </summary>
class Resource
{
public:
	Resource(ResourceManager* resourceManager)
		: _resourceManager(resourceManager) {};
	virtual ~Resource();

protected:
	ResourceManager* _resourceManager;
};

