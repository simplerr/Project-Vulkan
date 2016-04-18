#include "Timer.h"

namespace VulkanLib
{
	void Timer::FrameBegin()
	{
		frameBegin = std::chrono::high_resolution_clock::now();
	}

	uint32_t Timer::FrameEnd()
	{
		frameCounter++;
		auto frameEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(frameEnd - frameBegin).count();
		frameTimer = (float)tDiff / 1000.0f;

		// Convert to clamped timer value
		timer += timerSpeed * frameTimer;
		if (timer > 1.0)
		{
			timer -= 1.0f;
		}
		fpsTimer += (float)tDiff;

		// Increment frameCounter for 1 second, then update the FPS
		if (fpsTimer > 1000.0f)
		{
			framesPerSecond = frameCounter;
			fpsTimer = 0.0f;
			frameCounter = 0;

			return framesPerSecond;
		}

		return -1;
	}

	uint32_t Timer::GetFPS()
	{
		return framesPerSecond;
	}

	float Timer::GetElapsedTime()
	{
		return fpsTimer;
	}
}	// VulkanLib namespace