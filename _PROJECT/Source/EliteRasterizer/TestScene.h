#ifndef ELITE_TEST_SCENE
#define	ELITE_TEST_SCENE

#include "EScene.h"

namespace Elite
{
	class TriangleMesh;
	class TestScene final : public Scene
	{
	public:
		TestScene() = default;
		virtual ~TestScene() = default;

		TestScene(const TestScene&) = delete;
		TestScene(TestScene&&) noexcept = delete;
		TestScene& operator=(const TestScene&) = delete;
		TestScene& operator=(TestScene&&) noexcept = delete;

		void Initialize() override;
		void Update(Timer* pTimer) override;
	};
}

#endif