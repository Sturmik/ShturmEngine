#pragma once

#include <map>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

class AssetStore
{
public:
	static AssetStore& Get();

	void ClearAssets();

	void AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath);
	SDL_Texture* GetTexture(const std::string& assetId) const;

	void AddFont(const std::string& assetId, const std::string filePath, int fontSize);
	TTF_Font* GetFont(const std::string assetId);
private:
	AssetStore();
	~AssetStore();

	std::map<std::string, SDL_Texture*> _textures;
	std::map<std::string, TTF_Font*> _fonts;
};