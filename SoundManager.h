#pragma once

#include <SDL2/SDL_mixer.h>

class SoundManager
{
private:
	Mix_Chunk *uiConfirmSE = nullptr;

public:
	bool Init();
	void Shutdown();
	void PlayUiConfirm();
};
