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

class Block
{
public:
	int x = 500, y = 450;
	int width = 100, height = 100;

public:
	int Left() { return x; }
	int Right() { return x + width; }
	int Top() { return y; }
	int Bottom() { return y + height; }

	Block(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}

public:
	void Draw(SDL_Renderer *renderer)
	{
		SDL_Rect rect_block = {x, y, width, height};
		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // ブロックの色を灰色に設定
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
	void Update(int groundY, InputManager &inputManager, std::vector<Block> &blocks)
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

		if (y + height > groundY)
		{
			StayThisGround(groundY);
		}
	}

public:
	void Draw(SDL_Renderer *renderer)
	{
		SDL_Rect rect_player = {x, y, width, height}; // プレイヤーの描画だから結構ここで形をいじって大丈夫
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

class Ground
{
public:
	int x = 0, y = 600; // 地面の位置
	int width = 1200, height = 20; // 地面のサイズ

public:
	void Draw(SDL_Renderer *renderer)
	{
		SDL_Rect rect_ground = {x, y, width, height};
		SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // 地面の色を茶色に設定
		SDL_RenderFillRect(renderer, &rect_ground); // 地面を描く感じだね。
	}
};

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *window =
		SDL_CreateWindow("My Game", // 名前
						 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
						 1200, // 横
						 800, // 縦
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

	Ground ground;
	BackGround backGround;
	std::vector<Block> blocks = {
		{100, 500, 200, 20},
		{400, 400, 200, 20},
		{700, 300, 200, 20},
	};

	while (running)
	{ // 一応Update()的なやつだね。
		while (SDL_PollEvent(&event)) // 閉じるボタンが抑えれるか監視
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}
		backGround.Draw(
			renderer); // 背景を描く感じだね。これも毎回描いてるからUpdate()的なやつだね。

		player.Update(ground.y, inputManager, blocks);
		player.Draw(renderer);

		ground.Draw(renderer);
		for (Block &block : blocks)
		{
			block.Draw(renderer);
		}

		SDL_RenderPresent(
			renderer); // ここまで色々renrederをこねくりまわしたけどこいつを実行すると反映されます！最終的にこいつを書いてねって感じだね。
		SDL_Delay(16); // 16ms待つ感じだね。これで大体60fpsくらいになるはず！
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
