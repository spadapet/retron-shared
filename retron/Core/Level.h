#pragma once

#include "Components/PositionComponents.h"
#include "Core/GameSpec.h"

namespace ReTron
{
	class PositionComponents;

	class Level
	{
	public:
		Level(const DifficultySpec& diffSpec, const LevelSpec& levelSpec);

	private:
		void InitLevel();

		DifficultySpec _diffSpec;
		LevelSpec _levelSpec;
		entt::registry _registry;

		PositionComponents _positionComponents;
	};
}
