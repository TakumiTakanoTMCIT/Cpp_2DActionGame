#include "App.h"

#include <SDL2/SDL_image.h>

#include <iostream>

namespace
{
bool InitSDLSystems()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
	{
		std::cout << "SDL_image Init Error: " << IMG_GetError() << std::endl;
		SDL_Quit();
		return false;
	}

	if (TTF_Init() == -1)
	{
		std::cout << "SDL_ttf Init Error: " << TTF_GetError() << std::endl;
		IMG_Quit();
		SDL_Quit();
		return false;
	}

	return true;
}

bool CreateWindowAndRenderer(AppResources &resources, int screenWidth, int screenHeight)
{
	resources.window = SDL_CreateWindow("My Game",
										SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										screenWidth, screenHeight,
										SDL_WINDOW_SHOWN);
	if (resources.window == nullptr)
	{
		std::cout << "Window Error: " << SDL_GetError() << std::endl;
		return false;
	}

	resources.renderer = SDL_CreateRenderer(resources.window, -1, SDL_RENDERER_ACCELERATED);
	if (resources.renderer == nullptr)
	{
		std::cout << "Renderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(resources.window);
		resources.window = nullptr;
		return false;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	return true;
}

void InitAudioAndFont(AppResources &resources)
{
	if (!resources.soundManager.Init())
	{
		std::cout << "音はなしで続行するよ！" << std::endl;
	}

	resources.font = TTF_OpenFont("assets/BestTen-CRT.otf", 32);
	if (resources.font == nullptr)
	{
		std::cout << "Font Load Error: " << TTF_GetError() << std::endl;
	}
}

void LoadStageForGame(GameState &gameState)
{
	StageContext stageContext = CreateStage(gameState.currentStageIndex);
	gameState.stageData = stageContext.stageData;
	gameState.player.Reset(stageContext.playerStartX, stageContext.playerStartY);
	gameState.camera.Start(gameState.screenWidth, gameState.screenHeight);
}
} // namespace

bool InputManager::IsKeyPressed(SDL_Keycode key, SDL_Event &event) const
{
	if (event.type == SDL_KEYDOWN)
	{
		const Uint8 *state = SDL_GetKeyboardState(nullptr);
		return state[SDL_GetScancodeFromKey(key)];
	}
	return false;
}

bool InputManager::IsKeyHeld(SDL_Keycode key) const
{
	const Uint8 *state = SDL_GetKeyboardState(nullptr);
	return state[SDL_GetScancodeFromKey(key)];
}

void Camera::Start(int screenWidth, int screenHeight)
{
	x = 0;
	y = 0;
	startY = screenHeight / 2;
	startX = screenWidth / 2;
	bufferY = screenHeight / 2;
}

void Camera::Update(int playerX, int playerY)
{
	if (playerY < startY)
	{
		y = playerY - startY;
	}

	if (playerX > startX)
	{
		x = playerX - startX;
	}
}

void BackGround::Draw(SDL_Renderer *renderer) const
{
	SDL_SetRenderDrawColor(renderer, 40, 120, 140, 255);
	SDL_RenderClear(renderer);
}

void TitleScreen::Draw(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *font, const char *message) const
{
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 192, 205, 202, 100);
	SDL_Rect overlay = {0, 0, screenWidth, screenHeight};
	SDL_RenderFillRect(renderer, &overlay);

	if (font == nullptr)
	{
		return;
	}

	SDL_Color white = {255, 255, 255, 255};
	SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, message, white);
	if (textSurface == nullptr)
	{
		std::cout << "Text Surface Error: " << TTF_GetError() << std::endl;
		return;
	}

	SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	if (textTexture == nullptr)
	{
		std::cout << "Text Texture Error: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(textSurface);
		return;
	}

	SDL_Rect dstRect = {screenWidth / 2 - textSurface->w / 2, screenHeight / 2 - textSurface->h / 2, textSurface->w, textSurface->h};
	SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);

	SDL_DestroyTexture(textTexture);
	SDL_FreeSurface(textSurface);
}

bool InitializeApp(AppResources &resources, GameState &gameState)
{
	gameState.fallResetY = gameState.screenHeight + 400;

	if (!InitSDLSystems())
	{
		return false;
	}

	if (!CreateWindowAndRenderer(resources, gameState.screenWidth, gameState.screenHeight))
	{
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
		return false;
	}

	InitAudioAndFont(resources);

	gameState.camera.Start(gameState.screenWidth, gameState.screenHeight);
	gameState.player.LoadTexture(resources.renderer, "assets/player.png", 2);
	LoadStageForGame(gameState);

	SDL_SetWindowTitle(resources.window, "My Game - Press Space or Enter to Start");
	std::cout << "タイトル画面: Space か Enter でスタート" << std::endl;
	return true;
}

void ProcessEvents(AppResources &resources, GameState &gameState)
{
	auto playUiConfirm = [&resources]()
	{
		resources.soundManager.PlayUiConfirm();
	};

	while (SDL_PollEvent(&gameState.event))
	{
		if (gameState.event.type == SDL_QUIT)
		{
			gameState.running = false;
		}

		if (gameState.currentScene == GameScene::Title &&
			(gameState.inputManager.IsKeyPressed(SDLK_SPACE, gameState.event) ||
			 gameState.inputManager.IsKeyPressed(SDLK_RETURN, gameState.event)))
		{
			playUiConfirm();
			gameState.currentScene = GameScene::Playing;
			SDL_SetWindowTitle(resources.window, "My Game");
			std::cout << "ゲームスタート！" << std::endl;
		}

		if (gameState.currentScene == GameScene::Clear &&
			(gameState.inputManager.IsKeyPressed(SDLK_SPACE, gameState.event) ||
			 gameState.inputManager.IsKeyPressed(SDLK_RETURN, gameState.event)))
		{
			playUiConfirm();
			gameState.running = false;
		}
	}
}

void UpdateGame(AppResources &resources, GameState &gameState)
{
	if (gameState.currentScene != GameScene::Playing)
	{
		return;
	}

	gameState.player.Update(gameState.stageData, gameState.fallResetY);
	gameState.camera.Update(gameState.player.visualPixotX, gameState.player.visualPixotY);

	if (gameState.player.isDead)
	{
		std::cout << "死んだ！" << std::endl;
		LoadStageForGame(gameState);
	}
	else if (gameState.player.isGoal)
	{
		std::cout << "Goal!" << std::endl;
		if (gameState.currentStageIndex + 1 < GetStageCount())
		{
			gameState.currentStageIndex++;
			LoadStageForGame(gameState);
			std::cout << "次のステージへ！" << std::endl;
		}
		else
		{
			gameState.currentScene = GameScene::Clear;
			SDL_SetWindowTitle(resources.window, "My Game - Clear");
		}
	}
}

void RenderGame(AppResources &resources, GameState &gameState)
{
	gameState.backGround.Draw(resources.renderer);

	gameState.player.Draw(resources.renderer, gameState.camera.x, gameState.camera.y);
	for (Block &block : gameState.stageData.blocks)
	{
		block.Draw(resources.renderer, gameState.camera.x, gameState.camera.y);
	}

	for (Hazard &hazard : gameState.stageData.hazards)
	{
		hazard.Draw(resources.renderer, gameState.camera.x, gameState.camera.y);
		if (gameState.currentScene == GameScene::Playing)
		{
			hazard.Update(gameState.stageData.blocks);
		}
	}

	for (Goal &goal : gameState.stageData.goals)
	{
		goal.Draw(resources.renderer, gameState.camera.x, gameState.camera.y);
	}

	if (gameState.currentScene == GameScene::Title)
	{
		gameState.titleScreen.Draw(resources.renderer, gameState.screenWidth, gameState.screenHeight, resources.font, "PRESS SPACE TO START");
	}
	else if (gameState.currentScene == GameScene::Clear)
	{
		gameState.titleScreen.Draw(resources.renderer, gameState.screenWidth, gameState.screenHeight, resources.font, "GAME CLEAR! PRESS SPACE");
	}

	SDL_RenderPresent(resources.renderer);
}

void CleanupApp(AppResources &resources, GameState &gameState)
{
	gameState.player.UnloadTexture();
	resources.soundManager.Shutdown();

	if (resources.font != nullptr)
	{
		TTF_CloseFont(resources.font);
	}
	if (resources.renderer != nullptr)
	{
		SDL_DestroyRenderer(resources.renderer);
	}
	if (resources.window != nullptr)
	{
		SDL_DestroyWindow(resources.window);
	}

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}
