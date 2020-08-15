#include "pch.h"
#include "Core/Level.h"

ReTron::Level::Level(const DifficultySpec& diffSpec, const LevelSpec& levelSpec)
	: _diffSpec(diffSpec)
	, _levelSpec(levelSpec)
	, _positionComponents(_registry)
	, _collisionSystem(_registry, _positionComponents)
{
	InitLevel();
}

ReTron::Level::~Level()
{
}

void ReTron::Level::InitLevel()
{
}
