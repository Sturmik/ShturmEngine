#pragma once

#include <map>
#include <string>
#include <SDL3/SDL.h>

class AssetStore
{
public:
	static AssetStore& Get();

	void ClearAssets();
	void AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath);
	SDL_Texture* GetTexture(const std::string& assetId) const;

private:
	AssetStore();
	~AssetStore();

	std::map<std::string, SDL_Texture*> _textures;
};