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
	_texturesIds.clear();

	for (std::pair<const std::string, TTF_Font*> font : _fonts)
	{
		TTF_CloseFont(font.second);
	}
	_fonts.clear();

	for (std::pair<const std::string, SoundData>& sound : _sounds)
	{
		SDL_free(sound.second.buffer);
	}
	_sounds.clear();
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
	_texturesIds.push_back(assetId);

	LOG_INFO("Texture loaded: %s", assetId.c_str());
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId) const
{
	auto it = _textures.find(assetId);
	return (it != _textures.end()) ? it->second : nullptr;
}

const std::vector<std::string>& AssetStore::GetTextureIds() const
{
	return _texturesIds;
}

void AssetStore::AddFont(const std::string& assetId, const std::string filePath, int fontSize)
{
	TTF_Font* font = TTF_OpenFont(filePath.c_str(), fontSize);
	if (font == nullptr)
	{
		LOG_ERROR("Failed to load font %s : %s", filePath.c_str(), SDL_GetError());
		return;
	}

	_fonts.emplace(assetId, font);
	LOG_INFO("Font loaded: %s", assetId.c_str());
}

TTF_Font* AssetStore::GetFont(const std::string assetId)
{
	auto it = _fonts.find(assetId);
	return (it != _fonts.end()) ? it->second : nullptr;
}

void AssetStore::AddSound(const std::string& assetId, const std::string& filePath)
{
	SoundData sound;
	if (!SDL_LoadWAV(filePath.c_str(), &sound.spec, &sound.buffer, &sound.length))
	{
		LOG_ERROR("Failed to load sound %s : %s", filePath.c_str(), SDL_GetError());
		return;
	}

	_sounds.emplace(assetId, sound);
	LOG_INFO("Sound loaded: %s", assetId.c_str());
}

SoundData* AssetStore::GetSound(const std::string assetId)
{
	auto it = _sounds.find(assetId);
	return (it != _sounds.end()) ? &it->second : nullptr;
}
