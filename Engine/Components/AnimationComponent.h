#pragma once

#include <SDL3/SDL.h>

struct AnimationComponent
{
	AnimationComponent(int numFrames = 1, int frameRateSpeedPerSecond = 1, bool isLoop = true)
		: numFrames(numFrames), 
		frameRateSpeedPerSecond(frameRateSpeedPerSecond), 
		isLoop(isLoop), 
		currentFrame(1),
		startTime(SDL_GetTicks()){}

	int numFrames;
	int currentFrame;
	int frameRateSpeedPerSecond;
	bool isLoop;
	int startTime;
};