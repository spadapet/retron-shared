#include "pch.h"
#include "Core/Options.h"

Game::GameOptions::GameOptions()
	: _type(GameType::Arcade)
	, _players(GamePlayers::Single)
	, _difficulty(GameDifficulty::Normal)
{
}

Game::GameOptions::GameOptions(const GameOptions& rhs)
	: GameOptions()
{
	*this = rhs;
}

Game::GameOptions& Game::GameOptions::operator=(const GameOptions& rhs)
{
	std::memcpy(this, &rhs, sizeof(rhs));
	return *this;
}

Game::SystemOptions::SystemOptions()
	: _fullScreen(false)
	, _soundOn(true)
	, _musicOn(true)
	, _soundVolume(1)
	, _musicVolume(1)
{
}

Game::SystemOptions::SystemOptions(const SystemOptions& rhs)
	: SystemOptions()
{
	*this = rhs;
}

Game::SystemOptions& Game::SystemOptions::operator=(const SystemOptions& rhs)
{
	std::memcpy(this, &rhs, sizeof(rhs));
	return *this;
}
