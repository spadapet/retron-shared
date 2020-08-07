#pragma once

namespace ff
{
	class IResourceAccess;
}

namespace ReTron
{
	struct LevelSpec
	{
		int _grunts;
	};

	struct LevelSetSpec
	{
		ff::Vector<ff::String> _levels;
	};

	struct DifficultySpec
	{
		ff::String _name;
		ff::String _levelSet;
		int _lives;
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
