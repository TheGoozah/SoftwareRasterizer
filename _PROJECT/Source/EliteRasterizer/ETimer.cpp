#include "ETimer.h"
#include <SDL.h>

Elite::Timer::Timer()
{
	uint64_t countsPerSecond = SDL_GetPerformanceFrequency();
	m_SecondsPerCount = 1.0f / (float)countsPerSecond;
}

void Elite::Timer::Reset()
{
	uint64_t currentTime = SDL_GetPerformanceCounter();

	m_BaseTime = currentTime;
	m_PreviousTime = currentTime;
	m_StopTime = 0;
	m_FPSTimer = 0.0f;
	m_FPSCount = 0;
	m_IsStopped = false;
}

void Elite::Timer::Start()
{
	uint64_t startTime = SDL_GetPerformanceCounter();

	if (m_IsStopped)
	{
		m_PausedTime += (startTime - m_StopTime);

		m_PreviousTime = startTime;
		m_StopTime = 0;
		m_IsStopped = false;
	}
}

void Elite::Timer::Update()
{
	if (m_IsStopped)
	{
		m_FPS = 0;
		m_ElapsedTime = 0.0f;
		m_TotalTime = (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_BaseTime);
		return;
	}

	uint64_t currentTime = SDL_GetPerformanceCounter();
	m_CurrentTime = currentTime;

	m_ElapsedTime = (float)((m_CurrentTime - m_PreviousTime) * m_SecondsPerCount);
	m_PreviousTime = m_CurrentTime;

	if (m_ElapsedTime < 0.0f)
		m_ElapsedTime = 0.0f;

	if (m_ForceElapsedUpperBound && m_ElapsedTime > m_ElapsedUpperBound)
	{
		m_ElapsedTime = m_ElapsedUpperBound;
	}

	m_TotalTime = (float)(((m_CurrentTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);

	//FPS LOGIC
	m_FPSTimer += m_ElapsedTime;
	++m_FPSCount;
	if (m_FPSTimer >= 1.0f)
	{
		m_FPS = m_FPSCount;
		m_FPSCount = 0;
		m_FPSTimer -= 1.0f;
	}
}

void Elite::Timer::Stop()
{
	if (!m_IsStopped)
	{
		uint64_t currentTime = SDL_GetPerformanceCounter();

		m_StopTime = currentTime;
		m_IsStopped = true;
	}
}
