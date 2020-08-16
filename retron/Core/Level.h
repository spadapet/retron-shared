#pragma once

#include "Core/GameSpec.h"
#include "Systems/CollisionSystem.h"
#include "Systems/EntityManager.h"
#include "Systems/PositionSystem.h"

namespace ff
{
	class IRenderDepth;
	class IRenderer;
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

		void Advance(ff::AppGlobals* globals, ff::RectFixedInt cameraRect);
		void Render(ff::AppGlobals* globals, ff::IRenderer* render, ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect);

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
