#pragma once

#include <SDL3/SDL.h>

class Game
{
public:
	Game();
	~Game();
	void Initialize();
	void Run();
	void ProcessInput();
	void Update();
	void Render();
	void Destroy();

private:
	bool _isRunning;

	SDL_Window* _window;
	SDL_Renderer* _renderer;
};