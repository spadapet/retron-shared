#pragma once

#include "Core/GameSpec.h"
#include "Systems/CollisionSystem.h"
#include "Systems/PositionSystem.h"

namespace ReTron
{
	class PositionSystem;

	class Level
	{
	public:
		Level(const DifficultySpec& diffSpec, const LevelSpec& levelSpec);
		~Level();

	private:
		void InitLevel();

		DifficultySpec _diffSpec;
		LevelSpec _levelSpec;
		entt::registry _registry;

		PositionSystem _positionComponents;
		CollisionSystem _collisionSystem;
	};
}
