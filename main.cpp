#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

int currentStageIndex = 0;

const std::vector<std::vector<std::string>> kAllStages = {
	{
		"........B.....",
		"........B.....",
		".......B......",
		".......B......",
		"P...B.........",
		"..B.B......E.....................C",
		"GGGGGGGG..MMGGGGGGG.G.G.GMMGGGGGGG",
	},
	{
		"..............",
		"..............",
		"..............",
		"..............",
		"P....G....GG..",
		"....G......E.....................C",
		"GGGGGMMM..MMGGM.GGG.G.G.GMMGGGGGGG",
	},
};

class EventMediator
{
public:
	static std::function<void()> onPlayerStumpEnemy;

public:
	static void PlayerStumpEnemy()
	{
		if (onPlayerStumpEnemy)
		{
			onPlayerStumpEnemy();
		}
	}
};
std::function<void()> EventMediator::onPlayerStumpEnemy = nullptr;

class SoundManager
{
private:
	Mix_Chunk *uiConfirmSE = nullptr; // SEのファイルはここに入ります

public:
	// 初期化が上手くいったらtrueが帰ります。
	bool Init()
	{
		if ((Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3) == 0)
		{
			std::cout << "Mix_Init Error: " << Mix_GetError() << std::endl;
			return false;
		}

		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		{
			std::cout << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
			Mix_Quit();
			return false;
		}

		uiConfirmSE = Mix_LoadWAV("assets/決定ボタンを押す1.mp3");
		if (uiConfirmSE == nullptr)
		{
			std::cout << "Sound Load Error: " << Mix_GetError() << std::endl;
			Mix_CloseAudio();
			Mix_Quit();
			return false;
		}

		return true;
	}

	void Shutdown()
	{
		if (uiConfirmSE != nullptr)
		{
			Mix_FreeChunk(uiConfirmSE);
			uiConfirmSE = nullptr;
		}

		Mix_CloseAudio();
		Mix_Quit();
	}

	void PlayUiConfirm()
	{
		if (uiConfirmSE != nullptr)
		{
			Mix_PlayChannel(-1, uiConfirmSE, 0);
		}
	}
};

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
	int Left() const { return x; }
	int Right() const { return x + width; }
	int Top() const { return y; }
	int Bottom() const { return y + height; }
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

class Goal
{
public:
	int x = 0, y = 0;
	int width = 100, height = 100;

	Goal(int x, int y) : x(x), y(y) {}

	int Left() const { return x; }
	int Right() const { return x + width; }
	int Top() const { return y; }
	int Bottom() const { return y + height; }

	void Draw(SDL_Renderer *renderer, Camera &camera)
	{
		SDL_SetRenderDrawColor(renderer, 80, 220, 255, 255);
		SDL_Rect rect_goal = {x - camera.x, y - camera.y, width, height};
		SDL_RenderFillRect(renderer, &rect_goal);
	}
};

class Hazard
{
public:
	enum Type
	{
		Enemy,
		Magma,
	};

	int moveSpeed = 2;
	int x = 0, y = 0;
	int width = 100, height = 100;
	Type myType;
	bool isActive = true;

	Hazard(int x, int y, Type type) : x(x), y(y), myType(type) {}

	int Left() const { return x; }
	int Right() const { return x + width; }
	int Top() const { return y; }
	int Bottom() const { return y + height; }

	void Draw(SDL_Renderer *renderer, Camera &camera)
	{
		// そもそもDrawを実行しなかったら見えないので非アクティブにするというのが直感的に理解できます！
		if (!isActive)
		{
			return;
		}

		if (myType == Enemy)
		{
			SDL_SetRenderDrawColor(renderer, 220, 200, 40, 255);
		}
		else if (myType == Magma)
		{
			SDL_SetRenderDrawColor(renderer, 255, 90, 20, 255);
		}

		SDL_Rect rect_hazard = {x - camera.x, y - camera.y, width, height};
		SDL_RenderFillRect(renderer, &rect_hazard);
	}

public:
	void Remove()
	{
		isActive = false;
	}

private:
	bool IsStandingOnBlock(const Block &block, int checkX) const
	{
		bool isInsideX = checkX >= block.x && checkX < block.x + block.width;
		bool isOnTop = Bottom() == block.y;
		return isInsideX && isOnTop;
	}

	bool HasGroundAhead(const std::vector<Block> &blocks) const
	{
		int frontFootX = moveSpeed > 0 ? Right() : Left() - 1;
		for (const Block &block : blocks)
		{
			if (IsStandingOnBlock(block, frontFootX))
			{
				return true;
			}
		}
		return false;
	}

	bool WillHitWall(const std::vector<Block> &blocks, int nextX) const
	{
		for (const Block &block : blocks)
		{
			bool isOverlapX = nextX < block.x + block.width && nextX + width > block.x;
			bool isOverlapY = Bottom() > block.y && Top() < block.y + block.height;
			if (isOverlapX && isOverlapY)
			{
				return true;
			}
		}
		return false;
	}

public:
	void Update(const std::vector<Block> &blocks)
	{
		// 非アクティブなら動かさない、typeがEnemyじゃないなら動きません。
		if (!isActive || myType != Enemy)
		{
			return;
		}

		// 次のX座標を予測します！
		int nextX = x + moveSpeed;

		// 壁に当たるか地面がないなら反転します！
		if (WillHitWall(blocks, nextX) || !HasGroundAhead(blocks))
		{
			moveSpeed *= -1;
			nextX = x + moveSpeed;
		}

		// 次のX座標に壁がなければ移動します！
		if (!WillHitWall(blocks, nextX))
		{
			x = nextX;
		}
	}
};

struct StageData
{
	std::vector<Block> blocks;
	std::vector<Hazard> hazards;
	std::vector<Goal> goals;
};

struct StageContext
{
	StageData stageData;
	int playerStartX = 300;
	int playerStartY = 0;
};

enum class GameScene
{
	Title,
	Playing,
	Clear,
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

class TitleScreen
{
public:
	void Draw(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *font, const char *message)
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
		if (textSurface == nullptr) // ↑が失敗したときのためにnullチェック
		{
			std::cout << "Text Surface Error: " << TTF_GetError() << std::endl;
		}
		else
		{
			SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
			if (textTexture == nullptr) // 変換が失敗したときのためにnullチェック
			{
				std::cout << "Text Texture Error: " << SDL_GetError() << std::endl;
				SDL_FreeSurface(textSurface);
				return;
			}
			else
			{
				SDL_Rect dstRect = {screenWidth / 2 - textSurface->w / 2, screenHeight / 2 - textSurface->h / 2, textSurface->w, textSurface->h};
				SDL_RenderCopy(renderer, textTexture, nullptr, &dstRect);
				SDL_DestroyTexture(textTexture);
			}

			SDL_FreeSurface(textSurface);
		}
	}
};

class Player
{
public:
	int x = 300, y = 0; // プレイヤー画像の左上の位置
	int drawWidth, drawHeight;
	int colliderWidth, colliderHeight;
	int speed = 10;
	float yVelocity = 0.0f;
	float gravity = 0.5f;
	float maxFallSpeed = 12.0f;
	bool isGround = false;
	bool isDead = false;
	bool isGoal = false;
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

		// draw系は手を加えてはいけません、なぜならscale倍だけサイズを変えたいので変に数値をいじると縦長になったり平べったくなるので、当たり判定を変えるときにはcollider系をいじってください。
		drawWidth = surface->w * scale; // scaleが急に出てきますが、これは引数です。関数実行時にscaleに値を与えるとその倍率のサイズに変更します。高解像度のドット絵を入れるときに便利です。
		drawHeight = surface->h * scale;
		colliderWidth = drawWidth / 8; // 当たり判定は見た目より細くして、端の余白に引っかかりにくくしています。
		colliderHeight = drawHeight * 3 / 4; // 足元は合わせつつ、頭の上の余白は少し無視するイメージです。
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
		y = groundY - ColliderOffsetY() - colliderHeight; // 足元が地面にぴったり乗るように画像全体の位置を戻します。
		yVelocity = 0.0f;
		isGround = true;
	}

public:
	void Reset(int startX, int startY)
	{
		x = startX;
		y = startY;
		yVelocity = 0.0f;
		isGround = false;
		isDead = false;
		isGoal = false;
		visualPixotX = x + drawWidth / 2;
		visualPixotY = y + drawHeight / 2;
	}

	int ColliderOffsetX() { return (drawWidth - colliderWidth) / 2; }
	int ColliderOffsetY() { return drawHeight - colliderHeight; }

	int Left() { return x + ColliderOffsetX(); }
	int Right() { return Left() + colliderWidth; }
	int Top() { return y + ColliderOffsetY(); }
	int Bottom() { return Top() + colliderHeight; }

private:
	bool IsColliderInBlock(Block &block)
	{
		bool isOverlapX = Right() > block.Left() && Left() < block.Right();
		bool isOverlapY = Bottom() > block.Top() && Top() < block.Bottom();
		return isOverlapX && isOverlapY;
	}

public:
	bool IsColliderInHazard(Hazard &hazard)
	{
		// hazardが非アクティブであることは少ないように見えますが、死んだ敵は非アクティブになるので、その敵に触れてもうっかり死なないようにするためのチェックです。
		if (!hazard.isActive)
		{
			return false;
		}

		bool isOverlapX = Right() > hazard.Left() && Left() < hazard.Right();
		bool isOverlapY = Bottom() > hazard.Top() && Top() < hazard.Bottom();
		return isOverlapX && isOverlapY;
	}

	bool IsColliderInGoal(Goal &goal)
	{
		bool isOverlapX = Right() > goal.Left() && Left() < goal.Right();
		bool isOverlapY = Bottom() > goal.Top() && Top() < goal.Bottom();
		return isOverlapX && isOverlapY;
	}

public:
	void Update(InputManager &inputManager, std::vector<Block> &blocks, StageData &stageData, int fallResetY)
	{
		isDead = y > fallResetY;
		isGoal = false;

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
		int previousBottom = Bottom();
		y += static_cast<int>(yVelocity);

		for (Block &block : blocks)
		{
			if (IsColliderInBlock(block))
			{
				y = previousY;
				yVelocity = 0.0f;
				if (previousY + ColliderOffsetY() + colliderHeight <= block.Top())
				{
					isGround = true;
				}
			}
		}

		for (Hazard &hazard : stageData.hazards)
		{
			if (!IsColliderInHazard(hazard))
			{
				continue;
			}

			if (hazard.myType == Hazard::Enemy)
			{
				bool stomped = previousBottom <= hazard.Top() &&
					Bottom() >= hazard.Top() &&
					yVelocity > 0.0f;

				if (stomped)
				{
					std::cout << "敵を踏んだ！" << std::endl;
					hazard.Remove();
					Jump();
					EventMediator::PlayerStumpEnemy();
					continue;
				}
			}

			isDead = true;
		}

		for (Goal &goal : stageData.goals)
		{
			if (IsColliderInGoal(goal))
			{
				isGoal = true;
			}
		}

		visualPixotX = x + drawWidth / 2; // プレイヤー画像の中心のX座標を更新
		visualPixotY = y + drawHeight / 2; // プレイヤー画像の中心のY座標を更新
	}

public:
	void Draw(SDL_Renderer *renderer, Camera &camera)
	{
		SDL_Rect rect_player = {
			x - camera.x,
			y - camera.y,
			drawWidth,
			drawHeight};
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

StageContext CreateStage(int stageIndex)
{
	StageContext stageContext;
	const std::vector<std::string> &stage = kAllStages[stageIndex];

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
				stageContext.stageData.blocks.emplace_back(x, y, Block::Ground);
			}
			else if (tile == 'B')
			{
				stageContext.stageData.blocks.emplace_back(x, y, Block::Brick);
			}
			else if (tile == 'E')
			{
				stageContext.stageData.hazards.emplace_back(x, y, Hazard::Enemy);
			}
			else if (tile == 'M')
			{
				stageContext.stageData.hazards.emplace_back(x, y, Hazard::Magma);
			}
			else if (tile == 'C')
			{
				stageContext.stageData.goals.emplace_back(x, y);
			}
			else if (tile == 'P')
			{
				stageContext.playerStartX = x;
				stageContext.playerStartY = y;
			}
		}
	}

	return stageContext;
}

void LoadStage(int stageIndex, StageData &stageData, Player &player, Camera &camera, int screenWidth, int screenHeight)
{
	currentStageIndex = stageIndex;
	StageContext stageContext = CreateStage(stageIndex);
	stageData = stageContext.stageData;
	player.Reset(stageContext.playerStartX, stageContext.playerStartY);
	camera.Start(screenWidth, screenHeight);
}

int main()
{
	int screenWidth = 1200, screenHeight = 800;
	const int fallResetY = screenHeight + 400;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
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

	if (TTF_Init() == -1)
	{
		std::cout << "SDL_ttf Init Error: " << TTF_GetError() << std::endl;
		IMG_Quit();
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

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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

	SoundManager soundManager;
	bool hasAudio = soundManager.Init();
	if (!hasAudio)
	{
		std::cout << "音はなしで続行するよ！" << std::endl;
	}

	// フォントのロード
	TTF_Font *font = TTF_OpenFont("assets/BestTen-CRT.otf", 32);
	if (font == nullptr)
	{
		std::cout << "Font Load Error: " << TTF_GetError() << std::endl;
	}

	bool running = true;
	SDL_Event event;

	InputManager inputManager;
	Player player;
	Camera camera;
	BackGround backGround;
	TitleScreen titleScreen;
	StageData stageData;
	GameScene currentScene = GameScene::Title;
	camera.Start(screenWidth, screenHeight);
	player.LoadTexture(renderer, "assets/player.png", 2); // scaleで表示の倍率を設定できます。ドット絵のサイズ違いを入れるときには2にせず他の値を試してください。
	LoadStage(0, stageData, player, camera, screenWidth, screenHeight);
	EventMediator::onPlayerStumpEnemy = [&soundManager]() {};
	auto playUiConfirm = [&soundManager]()
	{
		soundManager.PlayUiConfirm();
	};
	SDL_SetWindowTitle(window, "My Game - Press Space or Enter to Start");
	std::cout << "タイトル画面: Space か Enter でスタート" << std::endl;

	while (running)
	{ // 一応Update()的なやつだね。
		while (SDL_PollEvent(&event)) // 閉じるボタンが抑えれるか監視
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}

			if (currentScene == GameScene::Title &&
				inputManager.isKeyPressed(SDLK_SPACE, event))
			{
				playUiConfirm();
				currentScene = GameScene::Playing;
				SDL_SetWindowTitle(window, "My Game");
				std::cout << "ゲームスタート！" << std::endl;
			}
			if (currentScene == GameScene::Title &&
				inputManager.isKeyPressed(SDLK_RETURN, event))
			{
				playUiConfirm();
				currentScene = GameScene::Playing;
				SDL_SetWindowTitle(window, "My Game");
				std::cout << "ゲームスタート！" << std::endl;
			}
			if (currentScene == GameScene::Clear &&
				inputManager.isKeyPressed(SDLK_SPACE, event))
			{
				playUiConfirm();
				running = false;
			}
			if (currentScene == GameScene::Clear &&
				inputManager.isKeyPressed(SDLK_RETURN, event))
			{
				playUiConfirm();
				running = false;
			}
		}
		backGround.Draw(renderer);

		if (currentScene == GameScene::Playing)
		{
			player.Update(inputManager, stageData.blocks, stageData, fallResetY);
			camera.Update(player.visualPixotX, player.visualPixotY);

			if (player.isDead)
			{
				std::cout << "死んだ！" << std::endl;
				LoadStage(currentStageIndex, stageData, player, camera, screenWidth, screenHeight);
			}
			else if (player.isGoal)
			{
				std::cout << "Goal!" << std::endl;
				if (currentStageIndex + 1 < kAllStages.size())
				{
					LoadStage(currentStageIndex + 1, stageData, player, camera, screenWidth, screenHeight);
					std::cout << "次のステージへ！" << std::endl;
				}
				else
				{
					currentScene = GameScene::Clear;
					SDL_SetWindowTitle(window, "My Game - Clear");
				}
			}
		}

		player.Draw(renderer, camera);
		for (Block &block : stageData.blocks)
		{
			block.Draw(renderer, camera);
		}
		for (Hazard &hazard : stageData.hazards)
		{
			hazard.Draw(renderer, camera);
			if (currentScene == GameScene::Playing)
			{
				hazard.Update(stageData.blocks);
			}
		}
		for (Goal &goal : stageData.goals)
		{
			goal.Draw(renderer, camera);
		}
		if (currentScene == GameScene::Title)
		{
			titleScreen.Draw(renderer, screenWidth, screenHeight, font, "PRESS SPACE TO START");
		}
		else if (currentScene == GameScene::Clear)
		{
			titleScreen.Draw(renderer, screenWidth, screenHeight, font, "GAME CLEAR! PRESS SPACE");
		}

		SDL_RenderPresent(renderer); // ここまで色々renrederをこねくりまわしたけどこいつを実行すると反映されます！最終的にこいつを書いてねって感じだね。
		SDL_Delay(16); // 16ms待つ感じだね。これで大体60fpsくらいになるはず！
	}

	player.UnloadTexture();
	soundManager.Shutdown();
	if (font != nullptr)
	{
		TTF_CloseFont(font);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
