#include <SDL.h>
#include <SDL_surface.h>
#undef main

#include <vld.h>
#include "ETimer.h"
#include "ECamera.h"
#include "ERenderer.h"
#include "EManager.h"
#include "EScene.h"
#include <iostream>

//Test scenes
#include "TestScene.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"Elite Rasterizer",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	Elite::Timer* pTimer = new Elite::Timer();
	Elite::Rasterizer::Camera camera = Elite::Rasterizer::Camera(Elite::FPoint3(0.f, 5.f, 65.f), 45.f, .1f, 100.f);
	Elite::Renderer* pRenderer = new Elite::Renderer(pWindow);
	Elite::Scene* pScene = nullptr;
	pScene = new Elite::TestScene();
	pScene->Initialize();

	//INFO
	std::cout
		<< "----------------------------------------------" << std::endl
		<< "Move camera: WSAD" << std::endl
		<< "Rotate camera: Right MouseButton" << std::endl
		<< "Fly through camera: Left MouseButton Drag" << std::endl
		<< "Up/Down camera: Right + Left MouseButton Drag" << std::endl
		<< "Toggle Depth Buffer Visualization: R" << std::endl
		<< "Take screenshot: T" << std::endl
		<< "----------------------------------------------" << std::endl;

	//Start loop
	pTimer->Start();
	float timer = 0;
	bool isLooping = true;
	bool takeScreenshot = false;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
					takeScreenshot = true;
				if (e.key.keysym.scancode == SDL_SCANCODE_R)
					pRenderer->ToggleDepthVisualization();
			}
		}

		//--------- Camera Controls ---------
		//For continuous events don't use PollEvent because that uses the repeat rate of the OS
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(0);
		//Key Movement
		float deltaTime = pTimer->GetElapsed();
		float cameraKeyboardSpeed = (pKeyboardState[SDL_SCANCODE_LSHIFT] + 1) * 10.f;
		camera.MoveForward((pKeyboardState[SDL_SCANCODE_S] - pKeyboardState[SDL_SCANCODE_W]) * cameraKeyboardSpeed * deltaTime);
		camera.MoveRight((pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) * cameraKeyboardSpeed * deltaTime);
		//Mouse Movement - UE4 like
		int x, y = 0;
		float cameraMouseSpeed = 4.0f;
		float rotationSensitivity = 0.075f;
		uint32_t mouseState = SDL_GetRelativeMouseState(&x, &y);
		if (mouseState == SDL_BUTTON_LMASK)
		{
			camera.MoveForward(y * cameraMouseSpeed * deltaTime);
			camera.Yaw(x * rotationSensitivity);
		}
		else if (mouseState == SDL_BUTTON_RMASK)
		{
			camera.Pitch(y * rotationSensitivity);
			camera.Yaw(x * rotationSensitivity);
		}
		else if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
		{
			camera.MoveUp(y * cameraMouseSpeed * deltaTime);
		}

		//--------- Update ---------
		pScene->Update(pTimer);

		//--------- Render ---------
		pRenderer->Update(pTimer);
		pRenderer->Render(camera);

		//--------- Timer ---------
		pTimer->Update();
		timer += pTimer->GetElapsed();
		if (timer >= 1.0f)
		{
			timer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

		//Save screenshot after full render
		if (takeScreenshot)
		{
			if (!pRenderer->SaveBackbufferToImage())
				std::cout << "Screenshot saved!" << std::endl;
			else
				std::cout << "Something went wrong. Screenshot not saved!" << std::endl;
			takeScreenshot = false;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pScene;
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}