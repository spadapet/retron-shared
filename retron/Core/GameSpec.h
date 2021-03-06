#pragma once

namespace ff
{
	class IResourceAccess;
}

namespace ReTron
{
	struct LevelRect
	{
		enum class Type
		{
			None,
			Bounds,
			Box,
			Safe,
			Objects,
		};

		Type _type;
		ff::RectFixedInt _rect;
	};

	struct LevelObjectsSpec : public LevelRect
	{
		size_t _electrode;
		size_t _grunt;
		size_t _hulk;
		size_t _bonusWoman;
		size_t _bonusMan;
		size_t _bonusChild;
	};

	struct LevelSpec
	{
		std::vector<LevelRect> _rects;
		std::vector<LevelObjectsSpec> _objects;
		ff::PointFixedInt _playerStart;
	};

	struct LevelSetSpec
	{
		std::vector<ff::String> _levels;
		size_t _loopStart;
	};

	struct DifficultySpec
	{
		ff::String _name;
		ff::String _levelSet;
		size_t _lives;
		size_t _gruntTickFrames; // grunts only move each "tick" number of frames
		size_t _gruntMaxTicks; // random 1-max ticks per move
		size_t _gruntMaxTicksRate; // frames to decrease max ticks
		size_t _gruntMinTicks;
		size_t _playerShotCounter;
		ff::FixedInt _gruntMove;
		ff::FixedInt _playerMove;
		ff::FixedInt _playerShotMove;
	};

	struct GameSpec
	{
		static GameSpec Load(ff::IResourceAccess* resources);

		bool _allowDebug;
		ff::FixedInt _joystickMin;
		ff::FixedInt _joystickMax;
		ff::Map<ff::String, DifficultySpec> _difficulties;
		ff::Map<ff::String, LevelSetSpec> _levelSets;
		ff::Map<ff::String, LevelSpec> _levels;
	};

	struct Player
	{
		Player* _coop;
		size_t _index;
		size_t _level;
		size_t _lives;
		size_t _score;
	};
}
