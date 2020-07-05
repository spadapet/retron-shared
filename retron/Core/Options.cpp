#include "pch.h"
#include "Core/Options.h"

NS_IMPLEMENT_REFLECTION_ENUM(ReTron::GameType, "ReTron.GameType")
{
	NsVal("Normal", ReTron::GameType::Normal);
	NsVal("InfiniteLives", ReTron::GameType::InfiniteLives);
	NsVal("NoBosses", ReTron::GameType::NoBosses);
}

NS_IMPLEMENT_REFLECTION_ENUM(ReTron::GamePlayers, "ReTron.GamePlayers")
{
	NsVal("One", ReTron::GamePlayers::One);
	NsVal("TwoTogether", ReTron::GamePlayers::TwoTogether);
	NsVal("TwoTakeTurns", ReTron::GamePlayers::TwoTakeTurns);
}

NS_IMPLEMENT_REFLECTION_ENUM(ReTron::GameDifficulty, "ReTron.GameDifficulty")
{
	NsVal("Baby", ReTron::GameDifficulty::Baby);
	NsVal("Easy", ReTron::GameDifficulty::Easy);
	NsVal("Normal", ReTron::GameDifficulty::Normal);
	NsVal("Hard", ReTron::GameDifficulty::Hard);
}

ReTron::GameOptions::GameOptions()
	: _version(GameOptions::CurrentVersion)
	, _type(GameType::Default)
	, _players(GamePlayers::Default)
	, _difficulty(GameDifficulty::Default)
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
	: _version(SystemOptions::CurrentVersion)
	, _fullScreen(false)
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
