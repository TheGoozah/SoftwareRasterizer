/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EDefaultManager.h: singleton that stores all the objects of type T in the scene
/*=============================================================================*/
#ifndef ELITE_DEFAULT_MANAGER
#define	ELITE_DEFAULT_MANAGER

#include <vector>
#include "ESingleton.h"

namespace Elite
{
	template<typename T>
	class Manager final : public Singleton<Manager<T>>
	{
	public:
		void AddObject(T* pObj)
		{
			m_Objects.push_back(pObj);
		}
		const std::vector<T*>& GetAllObjects() const
		{
			return m_Objects;
		}

		virtual ~Manager()
		{
			for (uint32_t i = 0, s = static_cast<uint32_t>(m_Objects.size()); i < s; ++i)
			{
				T* pObj = m_Objects.at(i);
				if (pObj)
					delete pObj;
			}
			m_Objects.clear();
		};

	private:
		std::vector<T*> m_Objects = {};
	};
}
#endif