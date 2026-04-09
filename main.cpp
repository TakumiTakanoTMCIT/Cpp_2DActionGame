#include "App.h"

int main()
{
	AppResources resources;
	GameState gameState;

	if (!InitializeApp(resources, gameState))
	{
		return 1;
	}

	while (gameState.running)
	{
		ProcessEvents(resources, gameState);
		UpdateGame(resources, gameState);
		RenderGame(resources, gameState);
		SDL_Delay(16);
	}

	CleanupApp(resources, gameState);
	return 0;
}
