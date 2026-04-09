#include "SoundManager.h"

#include <iostream>

bool SoundManager::Init()
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

void SoundManager::Shutdown()
{
	if (uiConfirmSE != nullptr)
	{
		Mix_FreeChunk(uiConfirmSE);
		uiConfirmSE = nullptr;
	}

	Mix_CloseAudio();
	Mix_Quit();
}

void SoundManager::PlayUiConfirm()
{
	if (uiConfirmSE != nullptr)
	{
		Mix_PlayChannel(-1, uiConfirmSE, 0);
	}
}
