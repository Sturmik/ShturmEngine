#pragma once

#include <SDL3/SDL.h>

const int FPS = 60;
const int MILLISECONDS_PER_FRAME = 1000 / FPS;

class Game
{
public:
	Game();
	~Game();
	void Initialize();
	void Setup();
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