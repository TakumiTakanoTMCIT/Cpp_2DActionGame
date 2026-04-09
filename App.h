#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Player.h"
#include "SoundManager.h"
#include "Stage.h"

enum class GameScene
{
	Title,
	Playing,
	Clear,
};

class InputManager
{
public:
	bool IsKeyPressed(SDL_Keycode key, SDL_Event &event) const;
	bool IsKeyHeld(SDL_Keycode key) const;
};

class Camera
{
public:
	int x = 0;
	int y = 0;
	int startX = 0;
	int startY = 0;
	int bufferY = 0;

public:
	void Start(int screenWidth, int screenHeight);
	void Update(int playerX, int playerY);
};

class BackGround
{
public:
	void Draw(SDL_Renderer *renderer) const;
};

class TitleScreen
{
public:
	void Draw(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *font, const char *message) const;
};

struct AppResources
{
	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;
	TTF_Font *font = nullptr;
	SoundManager soundManager;
};

struct GameState
{
	bool running = true;
	SDL_Event event{};
	InputManager inputManager;
	Player player;
	Camera camera;
	BackGround backGround;
	TitleScreen titleScreen;
	StageData stageData;
	GameScene currentScene = GameScene::Title;
	int currentStageIndex = 0;
	int screenWidth = 1200;
	int screenHeight = 800;
	int fallResetY = 1200;
};

bool InitializeApp(AppResources &resources, GameState &gameState);
void ProcessEvents(AppResources &resources, GameState &gameState);
void UpdateGame(AppResources &resources, GameState &gameState);
void RenderGame(AppResources &resources, GameState &gameState);
void CleanupApp(AppResources &resources, GameState &gameState);
