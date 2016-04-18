#include "Timer.h"

namespace VulkanLib
{
	void Timer::FrameBegin()
	{
		mFrameBegin = std::chrono::high_resolution_clock::now();
	}

	uint32_t Timer::FrameEnd()
	{
		mFrameCounter++;
		auto frameEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(frameEnd - mFrameBegin).count();
		mFrameTimer = (float)tDiff / 1000.0f;

		// Convert to clamped timer value
		mTimer += mTimerSpeed * mFrameTimer;
		if (mTimer > 1.0)
		{
			mTimer -= 1.0f;
		}
		mFpsTimer += (float)tDiff;

		// Increment frameCounter for 1 second, then update the FPS
		if (mFpsTimer > 1000.0f)
		{
			mFramesPerSecond = mFrameCounter;
			mFpsTimer = 0.0f;
			mFrameCounter = 0;

			return mFramesPerSecond;
		}

		return -1;
	}

	uint32_t Timer::GetFPS()
	{
		return mFramesPerSecond;
	}

	float Timer::GetElapsedTime()
	{
		return mFpsTimer;
	}
}	// VulkanLib namespace