#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "Game/Game.h"
#include "Logger/LoggerMacro.h"

Game g_game;

// SDL3 introduces it's own callbacks to handle init, game-loop and destroy of the game

// Here, we initialize our game
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	// Initialize logger
	Logger::Get().Initialize();

	// Initialize and setup the game
	g_game.Initialize();
	g_game.Setup();

	return SDL_APP_CONTINUE;
}

// This acts as part of cycle, where we update and render the world
SDL_AppResult SDL_AppIterate(void* appstate)
{
	g_game.Update();
	g_game.Render();

	return g_game.IsGameRunning() ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

// This acts as part of the cycle, which handles the input in the game
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	g_game.ProcessInput(*event);

	return SDL_APP_CONTINUE;
}

// This is program termination
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	g_game.Destroy();
}