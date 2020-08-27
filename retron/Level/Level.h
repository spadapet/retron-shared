#pragma once

#include "Core/GameSpec.h"
#include "Level/CollisionSystem.h"
#include "Level/EntityFactory.h"
#include "Level/EntitySystem.h"
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

	private:
		void InitLevel();
		void AdvancePlayer(entt::entity entity);
		void AdvanceGrunts();
		void HandleCollisions();

		ILevelService* _levelService;
		entt::registry _registry;
		EntitySystem _entitySystem;
		PositionSystem _positionSystem;
		CollisionSystem _collisionSystem;
		EntityFactory _entityFactory;
	};
}
