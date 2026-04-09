#pragma once

#include <SDL2/SDL.h>

#include <string>
#include <vector>

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
	Type myType;

public:
	Block(int x, int y, Type type);

	int Left() const;
	int Right() const;
	int Top() const;
	int Bottom() const;
	void Draw(SDL_Renderer *renderer, int cameraX, int cameraY) const;
};

class Goal
{
public:
	int x = 0, y = 0;
	int width = 100, height = 100;

public:
	Goal(int x, int y);

	int Left() const;
	int Right() const;
	int Top() const;
	int Bottom() const;
	void Draw(SDL_Renderer *renderer, int cameraX, int cameraY) const;
};

class Hazard
{
public:
	enum Type
	{
		Enemy,
		Magma,
	};

public:
	int moveSpeed = 2;
	int x = 0, y = 0;
	int width = 100, height = 100;
	Type myType;
	bool isActive = true;

public:
	Hazard(int x, int y, Type type);

	int Left() const;
	int Right() const;
	int Top() const;
	int Bottom() const;
	void Draw(SDL_Renderer *renderer, int cameraX, int cameraY) const;
	void Remove();
	void Update(const std::vector<Block> &blocks);

private:
	bool IsStandingOnBlock(const Block &block, int checkX) const;
	bool HasGroundAhead(const std::vector<Block> &blocks) const;
	bool WillHitWall(const std::vector<Block> &blocks, int nextX) const;
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

StageContext CreateStage(int stageIndex);
int GetStageCount();
