#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "Game/Game.h"
#include "Logger/LoggerMacro.h"

Game g_game;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	// Initialize logger
	Logger::Get().Initialize();

	// Initialize and setup the game
	g_game.Initialize();
	g_game.Setup();

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
	g_game.Update();
	g_game.Render();

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	g_game.ProcessInput();

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	g_game.Destroy();
}