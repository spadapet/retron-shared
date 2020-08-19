#pragma once

#include "Core/GameSpec.h"
#include "Level/CollisionSystem.h"
#include "Level/EntityManager.h"
#include "Level/PositionSystem.h"

namespace ff
{
	class IRenderDepth;
	class IRenderTarget;
}

namespace ReTron
{
	class ILevelService;

	class Level
	{
	public:
		Level(ILevelService* levelService);
		~Level();

		void Advance(ff::RectFixedInt cameraRect);
		void Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect);

		ff::PointFixedInt GetPlayerPosition(size_t index);

	private:
		void InitLevel();

		ILevelService* _levelService;
		entt::registry _registry;
		PositionSystem _positionComponents;
		CollisionSystem _collisionSystem;
		EntityManager _entityManager;
	};
}
