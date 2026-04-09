#include "Player.h"

#include <SDL2/SDL_image.h>

#include <iostream>

bool Player::LoadTexture(SDL_Renderer *renderer, const std::string &filePath, int scale)
{
	SDL_Surface *surface = IMG_Load(filePath.c_str());
	if (surface == nullptr)
	{
		std::cout << "Player sprite load error: " << IMG_GetError() << std::endl;
		return false;
	}

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (texture == nullptr)
	{
		std::cout << "Player texture create error: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(surface);
		return false;
	}

	drawWidth = surface->w * scale;
	drawHeight = surface->h * scale;
	colliderWidth = drawWidth / 8;
	colliderHeight = drawHeight * 3 / 4;
	SDL_FreeSurface(surface);
	return true;
}

void Player::UnloadTexture()
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}

void Player::Jump()
{
	yVelocity = -15.0f;
}

void Player::StayThisGround(int groundY)
{
	y = groundY - ColliderOffsetY() - colliderHeight;
	yVelocity = 0.0f;
	isGround = true;
}

void Player::Reset(int startX, int startY)
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

int Player::ColliderOffsetX() const { return (drawWidth - colliderWidth) / 2; }
int Player::ColliderOffsetY() const { return drawHeight - colliderHeight; }
int Player::Left() const { return x + ColliderOffsetX(); }
int Player::Right() const { return Left() + colliderWidth; }
int Player::Top() const { return y + ColliderOffsetY(); }
int Player::Bottom() const { return Top() + colliderHeight; }

bool Player::IsColliderInBlock(Block &block) const
{
	bool isOverlapX = Right() > block.Left() && Left() < block.Right();
	bool isOverlapY = Bottom() > block.Top() && Top() < block.Bottom();
	return isOverlapX && isOverlapY;
}

bool Player::IsColliderInHazard(Hazard &hazard) const
{
	if (!hazard.isActive)
	{
		return false;
	}

	bool isOverlapX = Right() > hazard.Left() && Left() < hazard.Right();
	bool isOverlapY = Bottom() > hazard.Top() && Top() < hazard.Bottom();
	return isOverlapX && isOverlapY;
}

bool Player::IsColliderInGoal(Goal &goal) const
{
	bool isOverlapX = Right() > goal.Left() && Left() < goal.Right();
	bool isOverlapY = Bottom() > goal.Top() && Top() < goal.Bottom();
	return isOverlapX && isOverlapY;
}

void Player::Update(StageData &stageData, int fallResetY)
{
	isDead = y > fallResetY;
	isGoal = false;

	const Uint8 *state = SDL_GetKeyboardState(NULL);
	bool wasGround = isGround;
	isGround = false;

	int previousX = x;
	if (state[SDL_GetScancodeFromKey(SDLK_LEFT)])
	{
		x -= speed;
	}
	if (state[SDL_GetScancodeFromKey(SDLK_RIGHT)])
	{
		x += speed;
	}

	for (Block &block : stageData.blocks)
	{
		if (IsColliderInBlock(block))
		{
			x = previousX;
		}
	}

	if (state[SDL_GetScancodeFromKey(SDLK_SPACE)] && wasGround)
	{
		Jump();
	}

	yVelocity += gravity;
	if (yVelocity > maxFallSpeed)
	{
		yVelocity = maxFallSpeed;
	}

	int previousY = y;
	int previousBottom = Bottom();
	y += static_cast<int>(yVelocity);

	for (Block &block : stageData.blocks)
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

	visualPixotX = x + drawWidth / 2;
	visualPixotY = y + drawHeight / 2;
}

void Player::Draw(SDL_Renderer *renderer, int cameraX, int cameraY)
{
	SDL_Rect rectPlayer = {
		x - cameraX,
		y - cameraY,
		drawWidth,
		drawHeight};
	if (texture != nullptr)
	{
		SDL_RenderCopy(renderer, texture, nullptr, &rectPlayer);
		return;
	}

	SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
	SDL_RenderFillRect(renderer, &rectPlayer);
	std::cout << "<Player,Draw>textureがnullです。";
}
