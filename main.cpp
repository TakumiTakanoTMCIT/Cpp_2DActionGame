#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <iostream>
#include <string>
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
	int x = 300, y = 0; // 四角形の位置
	int width = 128;
	int height = 128;
	int speed = 10;
	float yVelocity = 0.0f;
	float gravity = 0.5f;
	float maxFallSpeed = 12.0f;
	bool isGround = false;
	int visualPixotX, visualPixotY;
	SDL_Texture *texture = nullptr;

public:
	bool LoadTexture(SDL_Renderer *renderer, const std::string &filePath, int scale)
	{
		SDL_Surface *surface = IMG_Load(filePath.c_str()); // filepathのデータを読み込んでいます。
		if (surface == nullptr) // ↑で失敗したらnullptrが返ってくるのでここでnullチェックしてます
		{
			std::cout << "Player sprite load error: " << IMG_GetError() << std::endl;
			return false;
		}

		texture = SDL_CreateTextureFromSurface(renderer, surface); // surfaceは使いにくい？のでtextureに変換します。↓でまたnullチェックしています。
		if (texture == nullptr)
		{
			std::cout << "Player texture create error: " << SDL_GetError() << std::endl;
			SDL_FreeSurface(surface); // 変換に失敗したときにメモリに残さないでsurfaceを解放します。複雑に見えますがただのエラー時の処理なので定型文として覚えましょう。
			return false;
		}

		width = surface->w * scale; // scaleが急に出てきますが、これは引数です。関数実行時にscaleに値を与えるとその倍率のサイズに変更します。高解像度のドット絵を入れるときに便利です。
		height = surface->h * scale;
		SDL_FreeSurface(surface); // ↑のエラー時と同じメソッドですが、この時点で変換が完了したのでsurfaceを解放しましょう。
		return true;
	}

	void UnloadTexture() // Surfaceで作ったデータなどは自動で消えることはないので自前の関数で後処理を行います。main関数の最後の処理として実行します。これも癖強ポイントですが、定型文として覚えてください。
	{
		if (texture != nullptr)
		{
			SDL_DestroyTexture(texture);
			texture = nullptr;
		}
	}

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

		visualPixotX = x + width / 2; // プレイヤーの中心のX座標を更新
		visualPixotY = y + height / 2; // プレイヤーの中心のY座標を更新
	}

public:
	void Draw(SDL_Renderer *renderer, Camera &camera)
	{
		SDL_Rect rect_player = {x - camera.x, y - camera.y, width, height};
		if (texture != nullptr) // 一応nullチェックしています。
		{
			SDL_RenderCopy(renderer, texture, nullptr, &rect_player); // この関数がtextureを描画する本体です。rectplayerの位置とサイズで描画します。
			return;
		}

		// これらの処理は実行されないように見えますよね？なぜ記述したのかと言うと、もしtextureの読み込みに失敗したらプレイヤーが透明になるので一応短形を描画しています。
		SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
		SDL_RenderFillRect(renderer, &rect_player);
		std::cout << "<Player,Draw>textureがnullです。";
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

	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) // つまり正しくpngが使えるかのチェックです。定型文です。
	{
		std::cout << "SDL_image Init Error: " << IMG_GetError() << std::endl;
		SDL_Quit();
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
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	SDL_Renderer *renderer =
		SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		std::cout << "Renderer Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	// ドット絵ゲームなら必須の設定です。ぼやけないで表示する定型文。
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	bool running = true;
	SDL_Event event;

	InputManager inputManager;
	Player player;
	Camera camera;
	BackGround backGround;
	std::vector<Block> blocks = CreateStage(player);
	camera.Start(screenWidth, screenHeight);
	player.LoadTexture(renderer, "assets/player.png", 2); // scaleで表示の倍率を設定できます。ドット絵のサイズ違いを入れるときには2にせず他の値を試してください。

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

	player.UnloadTexture();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
