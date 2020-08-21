#pragma once

namespace ff
{
	class IResourceAccess;
}

namespace ReTron
{
	struct LevelObjectsSpec
	{
		ff::RectFixedInt _rect;
		size_t _electrode;
		size_t _grunt;
		size_t _hulk;
		size_t _bonusWoman;
		size_t _bonusMan;
		size_t _bonusChild;
	};

	struct LevelSpec
	{
		std::vector<ff::RectFixedInt> _bounds;
		std::vector<ff::RectFixedInt> _boxes;
		std::vector<LevelObjectsSpec> _objects;
		ff::PointFixedInt _playerStart;
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
