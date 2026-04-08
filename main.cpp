#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>

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

class Player
{
public:
    int r = 255, g = 80, b = 80; // 四角形の色

public:
    int x = 300, y = 0; // 四角形の位置

    public:
    int width = 200, height = 120; // 四角形のサイズ

    public:
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

public:
    void Update(int groundY, InputManager &inputManager)
    {
        if (inputManager.isKeyHeld(SDLK_LEFT))
        {
            x -= speed; // 左に移動
        }
        if (inputManager.isKeyHeld(SDLK_RIGHT))
        {
            x += speed; // 右に移動
        }
        if (inputManager.isKeyHeld(SDLK_SPACE) && isGround)
        {
            Jump();
        }

        yVelocity += gravity;
        if (yVelocity > maxFallSpeed)
        {
            yVelocity = maxFallSpeed;
        }

        y += static_cast<int>(yVelocity);

        if (y > groundY)
        {
            y = groundY;
            yVelocity = 0.0f;
            isGround = true;
        }
        else
        {
            isGround = false;
        }
    }

public:
    void Draw(SDL_Renderer *renderer)
    {
        SDL_Rect rect_player = {x, y - height, width, height}; //プレイヤーの描画だから結構ここで形をいじって大丈夫
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderFillRect(renderer, &rect_player); // rectを描くんだ〜！Fillだからまだわかりやすいかな？
    }
};

class BackGround{
    public:
    void Draw(SDL_Renderer *renderer)
    {
        SDL_SetRenderDrawColor(renderer, 40, 120, 140, 255);
        SDL_RenderClear(renderer);                         // 背景色で塗る感じだよね。まじで名前がわかりにくい、Fillとかにしやがれよ
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

class Block
{
    public:
    int x, y;
    int width = 100, height = 100;

    public:
    void Draw(SDL_Renderer *renderer)
    {
        SDL_Rect rect_block = {x, y, width, height};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // ブロックの色を灰色に設定
        SDL_RenderFillRect(renderer, &rect_block);
    }
};

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "My Game", // 名前
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1200, // 横
        800,  // 縦
        SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        std::cout << "Window Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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

    while (running)
    {                                 // 一応Update()的なやつだね。
        while (SDL_PollEvent(&event)) // 閉じるボタンが抑えれるか監視
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        backGround.Draw(renderer); // 背景を描く感じだね。これも毎回描いてるからUpdate()的なやつだね。

        player.Update(ground.y, inputManager);
        player.Draw(renderer);

        ground.Draw(renderer);

        SDL_RenderPresent(renderer); // ここまで色々renrederをこねくりまわしたけどこいつを実行すると反映されます！最終的にこいつを書いてねって感じだね。
        SDL_Delay(16);               // 16ms待つ感じだね。これで大体60fpsくらいになるはず！
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
