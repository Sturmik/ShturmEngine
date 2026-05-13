#include "AssetStore.h"

#include "Logger/LoggerMacro.h"

#include <SDL3_image/SDL_image.h>

AssetStore::AssetStore()
{
	LOG_INFO("AssetStore constructor called");
}

AssetStore::~AssetStore()
{
	LOG_INFO("AssetStore destructor called");
}

AssetStore& AssetStore::Get()
{
	static AssetStore assetStore;
	return assetStore;
}

void AssetStore::ClearAssets()
{
	for (std::pair<const std::string, SDL_Texture*> texture : _textures)
	{
		SDL_DestroyTexture(texture.second);
	}
	_textures.clear();

	for (std::pair<const std::string, TTF_Font*> font : _fonts)
	{
		TTF_CloseFont(font.second);
	}
	_fonts.clear();
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath)
{
	if (!SDL_GetPathInfo(filePath.c_str(), NULL)) 
	{
		LOG_ERROR("%s - file doesn't exist!", filePath.c_str());
		return;
	}

	SDL_Surface* surface = IMG_Load(filePath.c_str());
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);

	// Change texture scale mode to get rid of lines near edges
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

	// Add the texture to the map
	_textures.emplace(assetId, texture);

	LOG_INFO("New texture added to the Asset Store with id = %s", assetId.c_str());
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId) const
{
	return _textures.at(assetId);
}

void AssetStore::AddFont(const std::string& assetId, const std::string filePath, int fontSize)
{
	_fonts.emplace(assetId, TTF_OpenFont(filePath.c_str(), fontSize));
}

TTF_Font* AssetStore::GetFont(const std::string assetId)
{
	return _fonts.at(assetId);
}
