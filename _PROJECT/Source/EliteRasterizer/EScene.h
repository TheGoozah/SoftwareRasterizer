/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EScene.h: based class for scenes
/*=============================================================================*/
#ifndef ELITE_SCENE
#define	ELITE_SCENE

namespace Elite
{
	class Timer;
	class Scene
	{
	public:
		Scene() = default;
		virtual ~Scene() = default;

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(Timer* pTimer) = 0;
	};
}

#endif