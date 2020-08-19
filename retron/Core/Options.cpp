#include "pch.h"
#include "Core/Options.h"

static ff::StaticString DIFF_BABY(L"baby");
static ff::StaticString DIFF_EASY(L"easy");
static ff::StaticString DIFF_NORMAL(L"normal");
static ff::StaticString DIFF_HARD(L"hard");

NS_IMPLEMENT_REFLECTION_ENUM(ReTron::GameFlags, "ReTron.GameType")
{
	NsVal("Normal", ReTron::GameFlags::Normal);
	NsVal("InfiniteLives", ReTron::GameFlags::InfiniteLives);
	NsVal("NoBosses", ReTron::GameFlags::NoBosses);
}

NS_IMPLEMENT_REFLECTION_ENUM(ReTron::GamePlayers, "ReTron.GamePlayers")
{
	NsVal("One", ReTron::GamePlayers::One);
	NsVal("TwoTogether", ReTron::GamePlayers::TwoCoop);
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
	, _flags(GameFlags::Default)
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
	case GamePlayers::TwoCoop:
		return 2;
	}
}

bool ReTron::GameOptions::IsCoop() const
{
	return _players == GamePlayers::TwoCoop;
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
