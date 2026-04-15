#include "Game.h"

#include "Logger/LoggerMacro.h"

int main(int argc, char* argv[])
{
	// Initialize logger
	Logger::Get().Initialize();

	// Initialize game
	Game game;

	game.Initialize();
	game.Run();
	game.Destroy();

    return 0;
}
