#pragma once

namespace ff
{
	class IResourceAccess;
}

namespace ReTron
{
	struct LevelSpec
	{
		ff::PointFixedInt _playerStart;
		size_t _grunts;
	};

	struct LevelSetSpec
	{
		std::vector<ff::String> _levels;
	};

	struct DifficultySpec
	{
		ff::String _name;
		ff::String _levelSet;
		size_t _lives;
	};

	struct GameSpec
	{
		static GameSpec Load(ff::IResourceAccess* resources);

		bool _allowDebug;
		ff::Map<ff::String, DifficultySpec> _difficulties;
		ff::Map<ff::String, LevelSetSpec> _levelSets;
		ff::Map<ff::String, LevelSpec> _levels;
	};
}
