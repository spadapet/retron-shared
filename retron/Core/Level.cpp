#include "pch.h"
#include "Core/Level.h"

ReTron::Level::Level(const DifficultySpec& diffSpec, const LevelSpec& levelSpec)
	: _diffSpec(diffSpec)
	, _levelSpec(levelSpec)
	, _positionComponents(_registry)
{
	InitLevel();
}

void ReTron::Level::InitLevel()
{
}
