#include "pch.h"
#include "Core/Options.h"

ReTron::GameOptions::GameOptions()
	: _type(GameType::Arcade)
	, _players(GamePlayers::Single)
	, _difficulty(GameDifficulty::Normal)
{
}

ReTron::GameOptions::GameOptions(const GameOptions& rhs)
	: GameOptions()
{
	*this = rhs;
}

ReTron::GameOptions& ReTron::GameOptions::operator=(const GameOptions& rhs)
{
	std::memcpy(this, &rhs, sizeof(rhs));
	return *this;
}

ReTron::SystemOptions::SystemOptions()
	: _fullScreen(false)
	, _soundOn(true)
	, _musicOn(true)
	, _soundVolume(1)
	, _musicVolume(1)
{
}

ReTron::SystemOptions::SystemOptions(const SystemOptions& rhs)
	: SystemOptions()
{
	*this = rhs;
}

ReTron::SystemOptions& ReTron::SystemOptions::operator=(const SystemOptions& rhs)
{
	std::memcpy(this, &rhs, sizeof(rhs));
	return *this;
}
