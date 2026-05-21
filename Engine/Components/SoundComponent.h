#pragma once

#include <string>

struct SoundComponent
{
	std::string assetId;
	bool loop;
	float volume;

	SoundComponent(const std::string assetId = "", bool loop = false, float volume = 1.0f)
	 : assetId(assetId), loop(loop), volume(volume) {}
};