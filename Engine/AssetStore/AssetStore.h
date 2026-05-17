#pragma once

#include <map>
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_audio.h>

struct SoundData
{
	SDL_AudioSpec spec;
	Uint8* buffer = nullptr;
	Uint32 length = 0;
};

class AssetStore
{
public:
	static AssetStore& Get();

	void ClearAssets();

	// Textures
	void AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath);
	SDL_Texture* GetTexture(const std::string& assetId) const;
	const std::vector<std::string>& GetTextureIds() const;

	// Fonts
	void AddFont(const std::string& assetId, const std::string filePath, int fontSize);
	TTF_Font* GetFont(const std::string assetId);
	
	// Sounds
	void AddSound(const std::string& assetId, const std::string& filePath);
	SoundData* GetSound(const std::string assetId);
private:
	AssetStore();
	~AssetStore();

	// Textures
	std::map<std::string, SDL_Texture*> _textures;
	std::vector<std::string> _texturesIds;

	// Fonts
	std::map<std::string, TTF_Font*> _fonts;

	// Sounds
	std::map<std::string, SoundData> _sounds;
};