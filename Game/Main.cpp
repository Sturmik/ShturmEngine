#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "Game/Game.h"
#include "Logger/LoggerMacro.h"

#define TEST_LUA 0

#if TEST_LUA
int nativeCppCubeFunction(int n)
{
	return n * n * n;
}

#include <sol/sol.hpp>

void TestLua()
{
	sol::state lua;

	lua.open_libraries(sol::lib::base);

	// Expose and bind a native C++ function to be used by the Lua script
	lua["cube"] = nativeCppCubeFunction;

	lua.script_file("./Assets/Scripts/TestScript.lua");

	// A way to obtain a value from lua script
	int someVariableInsideCpp = lua["some_variable"];

	LOG_DEBUG("Some variable inside of lua script %d", someVariableInsideCpp);

	// A way to obtain values from lua tables and tables inside of it
	bool isFullScreen = lua["config"]["fullscreen"];
	LOG_DEBUG("Is FullScreen %d", (int)isFullScreen);

	sol::table config = lua["config"];
	int width = config["resolution"]["width"];
	int height = config["resolution"]["height"];
	LOG_DEBUG("Window size %dx%d", width, height);

	// A way to utilize lua functions in C++ code
	sol::function functionFactorial = lua["factorial"];
	int functionResult = functionFactorial(5);
	LOG_DEBUG("Factorial result %d", functionResult);
}
#endif

Game g_game;

// SDL3 introduces it's own callbacks to handle init, game-loop and destroy of the game

// Here, we initialize our game
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	// Initialize logger
	Logger::Get().Initialize();

#if TEST_LUA
	TestLua();
#endif

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