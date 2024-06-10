#pragma once
#include "framework.h"
#include "Object.h"


namespace Render
{
	class ResourceManager
	{
	public:
				/// <summary>
		/// AssetNum, Asset
		/// </summary>
		/// <typeparam name="T"></typeparam>
		template<typename T>
		static std::map<UINT, std::shared_ptr<T>> m_Assets;

		template<typename T>
		std::weak_ptr<T> GetAsset(const UINT _AssetNum)
		{
			//모델이있다면 모델을 반환한다.
			if (m_Assets<T>.find(_AssetNum) != m_Assets<T>.end())
			{
				return m_Assets<T>[_AssetNum];
			}
			std::shared_ptr<T> result;
			return result;
		}
		template<typename T>
		const std::map<UINT, std::shared_ptr<T>>& GetAllAssets()
		{
			return m_Assets<T>;
		}

		template<typename T>
		void AddAsset(const UINT _AssetNum,std::shared_ptr<T>& _Asset)
		{
			if (m_Assets<T>.find(_AssetNum) != m_Assets<T>.end())
			{
				return;
			}
			m_Assets<T>[_AssetNum] = _Asset;
		}




	public:
		ResourceManager() {}
		~ResourceManager() {}
	public:

		template <typename T>
		bool GetResource(const std::string& _type, const std::string& _name, std::weak_ptr<T>& _data)
		{
			if (_type != typeid(T).name())
			{
				return false;
			}
			if (m_ResourceContainer.find(_type) == m_ResourceContainer.end())
			{
				return false;
			}
			else if (m_ResourceContainer[_type].find(_name) == m_ResourceContainer[_type].end())
			{
				return false;
			}

			_data = std::reinterpret_pointer_cast<T>(m_ResourceContainer[_type][_name]);
			return true;
		}

		// need change type
		std::map <std::string, std::shared_ptr<void>>& GetAllResources(const std::string& _type)
		{
			return m_ResourceContainer[_type];
		}

		template <typename T>
		bool AddResource(const std::string& _type, const std::string& _name, T& _data)
		{

			if (m_ResourceContainer.find(_type) != m_ResourceContainer.end())
			{
				if (m_ResourceContainer[_type].find(_name) != m_ResourceContainer[_type].end())
				{
					return false;
				}
			}

			std::shared_ptr<T> src = std::make_shared <T>(_data);
			m_ResourceContainer[_type][_name] = std::reinterpret_pointer_cast<void>(src);
			return true;
		}

		void AddMaterial(UINT _Index, std::shared_ptr<Material> _Data)
		{
			// 이미 머티리얼이 있다면 추가하지 않는다.
			if (m_MaterialContainer.find(_Index) != m_MaterialContainer.end())
			{
				return;
			}
			m_MaterialContainer[_Index] = _Data;
		}

		void AddMaterial(UINT _Index, const Material& _Data)
		{
			// 이미 머티리얼이 있다면 추가하지 않는다.
			if (m_MaterialContainer.find(_Index) != m_MaterialContainer.end())
			{
				return;
			}
			m_MaterialContainer[_Index] = std::make_shared<Material>(_Data);
		}

		std::weak_ptr<Material> GetMaterial(UINT _Index)
		{
			if (m_MaterialContainer.find(_Index) != m_MaterialContainer.end())
			{
				return m_MaterialContainer[_Index];
			}
			return std::shared_ptr<Material>();
		}
	public:
		std::map<std::string, std::map <std::string, std::shared_ptr<void>>> m_ResourceContainer;
		std::map<UINT, std::shared_ptr<Material>> m_MaterialContainer;
	};

	template<typename T>
	std::map<UINT, std::shared_ptr<T>> ResourceManager::m_Assets;
}