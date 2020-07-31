#include "pch.h"
#include "Core/Options.h"

static ff::StaticString DIFF_BABY(L"baby");
static ff::StaticString DIFF_EASY(L"easy");
static ff::StaticString DIFF_NORMAL(L"normal");
static ff::StaticString DIFF_HARD(L"hard");

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

ff::StringRef ReTron::GameOptions::GetDifficultyId() const
{
	switch (_difficulty)
	{
	case GameDifficulty::Baby:
		return ::DIFF_BABY;

	case GameDifficulty::Easy:
		return ::DIFF_EASY;

	default:
	case GameDifficulty::Normal:
		return ::DIFF_NORMAL;

	case GameDifficulty::Hard:
		return ::DIFF_HARD;
	}
}

size_t ReTron::GameOptions::GetPlayerCount() const
{
	switch (_players)
	{
	default:
		return 1;

	case GamePlayers::TwoTakeTurns:
	case GamePlayers::TwoTogether:
		return 2;
	}
}

bool ReTron::GameOptions::ArePlayersTogether() const
{
	return _players == GamePlayers::TwoTogether;
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
