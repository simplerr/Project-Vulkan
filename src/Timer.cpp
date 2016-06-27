#include "Timer.h"
#include <fstream>

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
		mLifetimeTimer += (float)tDiff;

		// Increment frameCounter for 1 second, then update the FPS
		if (mFpsTimer > 1000.0f)
		{
			mFramesPerSecond = mFrameCounter;
			mFpsTimer = 0.0f;
			mFrameCounter = 0;

			// Add FPS to the log
			mFpsLog.push_back(mFramesPerSecond);

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
	void Timer::PrintLog(std::ofstream& fout)
	{
		fout << "Capture time: " << mLifetimeTimer / 1000.0f << " seconds" << std::endl;

		float sum = 0.0f;
		for (int i = 0; i < mFpsLog.size(); i++)
		{
			//fout << mFpsLog[i] << std::endl;
			sum += mFpsLog[i];
		}

		fout << "Average FPS: " << sum / mFpsLog.size() << std::endl;
	}
}	// VulkanLib namespace