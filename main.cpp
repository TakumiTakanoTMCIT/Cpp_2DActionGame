#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <vector>

class InputManager
{
public:
	bool isKeyPressed(SDL_Keycode key, SDL_Event &event)
	{
		if (event.type == SDL_KEYDOWN)
		{
			const Uint8 *state = SDL_GetKeyboardState(NULL);
			return state[SDL_GetScancodeFromKey(key)];
		}
		return false;
	}

	bool isKeyHeld(SDL_Keycode key)
	{
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		return state[SDL_GetScancodeFromKey(key)];
	}
};

class Camera
{
public:
	int x;
	int y;
	int startX, startY;
	int bufferY;

public:
	void Start(int screenWidth, int screenHeight)
	{
		x = 0;
		y = 0;
		startY = screenHeight / 2;
		startX = screenWidth / 2;
		bufferY = screenHeight / 2;
	}

public:
	void Update(int playerX, int playerY)
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
};

class Block
{
public:
	enum Type
	{
		Ground,
		Brick,
	};

public:
	int x = 500, y = 450;
	int width = 100, height = 100;

public:
	int Left() { return x; }
	int Right() { return x + width; }
	int Top() { return y; }
	int Bottom() { return y + height; }
	Type myType;

	// コンストラクタ
	Block(int x, int y, Type type) : x(x), y(y), myType(type) {}

public:
	void Draw(SDL_Renderer *renderer, Camera &camera)
	{
		if (myType == Ground)
		{
			SDL_SetRenderDrawColor(renderer, 80, 160, 80, 255); // 地面の色を緑色に設定
		}
		else if (myType == Brick)
		{
			SDL_SetRenderDrawColor(renderer, 160, 80, 80, 255); // ブロックの色を赤色に設定
		}

		SDL_Rect rect_block = {x - camera.x, y - camera.y, width, height};
		SDL_RenderFillRect(renderer, &rect_block);
	}
};

class Player
{
public:
	int r = 255, g = 80, b = 80; // 四角形の色
	int x = 300, y = 0; // 四角形の位置
	int width = 200, height = 120; // 四角形のサイズ
	int speed = 10;
	float yVelocity = 0.0f;
	float gravity = 0.5f;
	float maxFallSpeed = 12.0f;
	bool isGround = false;
	int visualPixotX, visualPixotY;

private:
	void Jump()
	{
		yVelocity = -15.0f; // ジャンプの初速
	}

private:
	void StayThisGround(int groundY)
	{
		y = groundY - height; // プレイヤーの下端が地面に接するように位置を調整
		yVelocity = 0.0f;
		isGround = true;
	}

public:
	int Left() { return x; }
	int Right() { return x + width; }
	int Top() { return y; }
	int Bottom() { return y + height; }

private:
	bool IsColliderInBlock(Block &block)
	{
		bool isOverlapX = Right() > block.Left() && Left() < block.Right();
		bool isOverlapY = Bottom() > block.Top() && Top() < block.Bottom();
		return isOverlapX && isOverlapY;
	}

public:
	void Update(InputManager &inputManager, std::vector<Block> &blocks)
	{
		bool wasGround = isGround;
		isGround = false;

		int previousX = x;
		if (inputManager.isKeyHeld(SDLK_LEFT))
		{
			x -= speed; // 左に移動
		}
		if (inputManager.isKeyHeld(SDLK_RIGHT))
		{
			x += speed; // 右に移動
		}

		for (Block &block : blocks)
		{
			if (IsColliderInBlock(block))
			{
				x = previousX;
			}
		}

		if (inputManager.isKeyHeld(SDLK_SPACE) && wasGround)
		{
			Jump();
			wasGround = false;
		}

		yVelocity += gravity;
		if (yVelocity > maxFallSpeed)
		{
			yVelocity = maxFallSpeed;
		}

		int previousY = y;
		y += static_cast<int>(yVelocity);

		for (Block &block : blocks)
		{
			if (IsColliderInBlock(block))
			{
				y = previousY;
				yVelocity = 0.0f;
				if (previousY + height <= block.Top())
				{
					isGround = true;
				}
			}
		}

		/*
		if (y + height > groundY)
		{
			StayThisGround(groundY);
		}*/

		visualPixotX = x + width / 2; // プレイヤーの中心のX座標を更新
		visualPixotY = y + height / 2; // プレイヤーの中心のY座標を更新
	}

public:
	void Draw(SDL_Renderer *renderer, Camera &camera)
	{
		SDL_Rect rect_player = {x - camera.x, y - camera.y, width, height}; // プレイヤーの描画だから結構ここで形をいじって大丈夫
		SDL_SetRenderDrawColor(renderer, r, g, b, 255);
		SDL_RenderFillRect(renderer, &rect_player); // rectを描くんだ〜！Fillだからまだわかりやすいかな？
	}
};

class BackGround
{
public:
	void Draw(SDL_Renderer *renderer)
	{
		SDL_SetRenderDrawColor(renderer, 40, 120, 140, 255);
		SDL_RenderClear(renderer); // 背景色で塗る感じだよね。まじで名前がわかりにくい、Fillとかにしやがれよ
	}
};

std::vector<Block> CreateStage(Player &player)
{
	std::vector<Block> blocks;

	std::vector<std::string> stage = {
		"........B.",
		"........B.",
		".......B..",
		"..........",
		"....B.....",
		"..B.B.....",
		"GGGGGGGGGG",
	};

	int blockSize = 100;

	for (int row = 0; row < stage.size(); row++)
	{
		for (int col = 0; col < stage[row].size(); col++)
		{
			char tile = stage[row][col];
			int x = col * blockSize;
			int y = row * blockSize;

			if (tile == 'G')
			{
				blocks.emplace_back(x, y, Block::Ground);
			}
			else if (tile == 'B')
			{
				blocks.emplace_back(x, y, Block::Brick);
			}
		}
	}

	return blocks;
}

int main()
{
	int screenWidth = 1200, screenHeight = 800;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *window =
		SDL_CreateWindow("My Game", // 名前
						 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
						 screenWidth, // 横
						 screenHeight, // 縦
						 SDL_WINDOW_SHOWN);

	if (window == nullptr)
	{
		std::cout << "Window Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	SDL_Renderer *renderer =
		SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		std::cout << "Renderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	bool running = true;
	SDL_Event event;

	InputManager inputManager;
	Player player;
	Camera camera;
	BackGround backGround;
	std::vector<Block> blocks = CreateStage(player);
	camera.Start(screenWidth, screenHeight);

	while (running)
	{ // 一応Update()的なやつだね。
		while (SDL_PollEvent(&event)) // 閉じるボタンが抑えれるか監視
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}
		backGround.Draw(renderer);

		player.Update(inputManager, blocks);
		camera.Update(player.visualPixotX, player.visualPixotY);

		player.Draw(renderer, camera);
		for (Block &block : blocks)
		{
			block.Draw(renderer, camera);
		}

		SDL_RenderPresent(renderer); // ここまで色々renrederをこねくりまわしたけどこいつを実行すると反映されます！最終的にこいつを書いてねって感じだね。
		SDL_Delay(16); // 16ms待つ感じだね。これで大体60fpsくらいになるはず！
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
