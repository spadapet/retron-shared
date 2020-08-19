#pragma once

namespace ReTron
{
	enum class GameFlags
	{
		Normal = 0x00,
		InfiniteLives = 0x01,
		NoBosses = 0x02,

		Default = Normal
	};

	enum class GamePlayers
	{
		One = 0,
		TwoTakeTurns = 1,
		TwoCoop = 2,

		Default = One
	};

	enum class GameDifficulty
	{
		Baby = 0,
		Easy = 1,
		Normal = 2,
		Hard = 3,

		Default = Normal
	};

	struct GameOptions
	{
		GameOptions();

		ff::StringRef GetDifficultyId() const;
		size_t GetPlayerCount() const;
		bool IsCoop() const;

		static const int CurrentVersion = 1;

		int _version;
		GameFlags _flags;
		GamePlayers _players;
		GameDifficulty _difficulty;
	};

	struct SystemOptions
	{
		SystemOptions();

		static const int CurrentVersion = 1;

		int _version;
		bool _fullScreen;
		bool _soundOn;
		bool _musicOn;
		ff::FixedInt _soundVolume;
		ff::FixedInt _musicVolume;
	};
}

NS_DECLARE_REFLECTION_ENUM(ReTron::GameFlags)
NS_DECLARE_REFLECTION_ENUM(ReTron::GamePlayers)
NS_DECLARE_REFLECTION_ENUM(ReTron::GameDifficulty)
