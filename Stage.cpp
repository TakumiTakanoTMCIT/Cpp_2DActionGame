#include "Stage.h"

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

Block::Block(int x, int y, Type type) : x(x), y(y), myType(type) {}

int Block::Left() const { return x; }
int Block::Right() const { return x + width; }
int Block::Top() const { return y; }
int Block::Bottom() const { return y + height; }

void Block::Draw(SDL_Renderer *renderer, int cameraX, int cameraY) const
{
	if (myType == Ground)
	{
		SDL_SetRenderDrawColor(renderer, 80, 160, 80, 255);
	}
	else if (myType == Brick)
	{
		SDL_SetRenderDrawColor(renderer, 160, 80, 80, 255);
	}

	SDL_Rect rectBlock = {x - cameraX, y - cameraY, width, height};
	SDL_RenderFillRect(renderer, &rectBlock);
}

Goal::Goal(int x, int y) : x(x), y(y) {}

int Goal::Left() const { return x; }
int Goal::Right() const { return x + width; }
int Goal::Top() const { return y; }
int Goal::Bottom() const { return y + height; }

void Goal::Draw(SDL_Renderer *renderer, int cameraX, int cameraY) const
{
	SDL_SetRenderDrawColor(renderer, 80, 220, 255, 255);
	SDL_Rect rectGoal = {x - cameraX, y - cameraY, width, height};
	SDL_RenderFillRect(renderer, &rectGoal);
}

Hazard::Hazard(int x, int y, Type type) : x(x), y(y), myType(type) {}

int Hazard::Left() const { return x; }
int Hazard::Right() const { return x + width; }
int Hazard::Top() const { return y; }
int Hazard::Bottom() const { return y + height; }

void Hazard::Draw(SDL_Renderer *renderer, int cameraX, int cameraY) const
{
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

	SDL_Rect rectHazard = {x - cameraX, y - cameraY, width, height};
	SDL_RenderFillRect(renderer, &rectHazard);
}

void Hazard::Remove()
{
	isActive = false;
}

bool Hazard::IsStandingOnBlock(const Block &block, int checkX) const
{
	bool isInsideX = checkX >= block.x && checkX < block.x + block.width;
	bool isOnTop = Bottom() == block.y;
	return isInsideX && isOnTop;
}

bool Hazard::HasGroundAhead(const std::vector<Block> &blocks) const
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

bool Hazard::WillHitWall(const std::vector<Block> &blocks, int nextX) const
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

void Hazard::Update(const std::vector<Block> &blocks)
{
	if (!isActive || myType != Enemy)
	{
		return;
	}

	int nextX = x + moveSpeed;
	if (WillHitWall(blocks, nextX) || !HasGroundAhead(blocks))
	{
		moveSpeed *= -1;
		nextX = x + moveSpeed;
	}

	if (!WillHitWall(blocks, nextX))
	{
		x = nextX;
	}
}

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

int GetStageCount()
{
	return static_cast<int>(kAllStages.size());
}
