/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ESingleton.h: template class to create singletons in the Engine.
/*=============================================================================*/
#ifndef ELITE_SINGLETON
#define	ELITE_SINGLETON

namespace Elite
{
	template<typename T>
	class Singleton
	{
	public:
		//=== Public Functions ===
		static T* GetInstance() 
		{
			if (!m_pInstance)
				m_pInstance = new T();
			return m_pInstance;
		};
		static void Destroy() 
		{ 
			delete m_pInstance; 
			m_pInstance = nullptr;
		};

	protected:
		//=== Constructors & Destructor
		Singleton() = default;
		virtual ~Singleton() = default;

		//=== Datamembers ===
		static T* m_pInstance;

	private:
		Singleton(Singleton const&) {};
		Singleton& operator=(Singleton const&) {};
	};

	template<typename T>
	typename T* Singleton<T>::m_pInstance = 0;
}
#endif