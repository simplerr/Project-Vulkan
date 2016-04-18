#pragma once
#include <chrono>
#include <cstdint>

namespace VulkanLib
{
	class Timer
	{
	public:
		void FrameBegin();
		uint32_t FrameEnd();			// Returns the FPS if 1000.0f milliseconds have passed

		uint32_t GetFPS();
		float GetElapsedTime();
	private:
		std::chrono::high_resolution_clock::time_point frameBegin;

		float frameTimer = 1.0f;		// Last frame time, measured using a high performance timer (if available)
		uint32_t frameCounter = 0;		// Frame counter to display fps
		float timer = 0.0f;				// Defines a frame rate independent timer value clamped from -1.0...1.0
		float timerSpeed = 0.25f;		// Multiplier for speeding up (or slowing down) the global timer
		float fpsTimer = 0.0f;			// FPS timer (one second interval)
		uint32_t framesPerSecond;
	};
}	// VulkanLib namespace