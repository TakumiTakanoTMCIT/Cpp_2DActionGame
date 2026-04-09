#pragma once

#include <SDL2/SDL.h>

#include <string>

#include "Stage.h"

class Player
{
public:
	int x = 300, y = 0;
	int drawWidth = 0, drawHeight = 0;
	int colliderWidth = 0, colliderHeight = 0;
	int speed = 10;
	float yVelocity = 0.0f;
	float gravity = 0.5f;
	float maxFallSpeed = 12.0f;
	bool isGround = false;
	bool isDead = false;
	bool isGoal = false;
	int visualPixotX = 0, visualPixotY = 0;
	SDL_Texture *texture = nullptr;

public:
	bool LoadTexture(SDL_Renderer *renderer, const std::string &filePath, int scale);
	void UnloadTexture();
	void Reset(int startX, int startY);
	void Update(StageData &stageData, int fallResetY);
	void Draw(SDL_Renderer *renderer, int cameraX, int cameraY);

	int ColliderOffsetX() const;
	int ColliderOffsetY() const;
	int Left() const;
	int Right() const;
	int Top() const;
	int Bottom() const;

private:
	void Jump();
	void StayThisGround(int groundY);
	bool IsColliderInBlock(Block &block) const;
	bool IsColliderInHazard(Hazard &hazard) const;
	bool IsColliderInGoal(Goal &goal) const;
};
