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
		entt::entity CreateObject(EntityType type, const ff::RectFixedInt& rect, const std::vector<ff::RectFixedInt>& boxes);

		void AdvanceEntity(entt::entity entity, EntityType type);
		void AdvancePlayer(entt::entity entity);
		void AdvanceGrunts();
		void AdvanceCollisions();

		void RenderEntity(entt::entity entity, EntityType type, ff::PixelRendererActive& render);
		void RenderPlayer(entt::entity entity, ff::PixelRendererActive& render);
		void RenderBonus(entt::entity entity, EntityType type, ff::PixelRendererActive& render);
		void RenderElectrode(entt::entity entity, ff::PixelRendererActive& render);
		void RenderHulk(entt::entity entity, ff::PixelRendererActive& render);
		void RenderGrunt(entt::entity entity, ff::PixelRendererActive& render);

		template<typename... Args>
		void EnumEntities(entt::delegate<void(entt::entity, EntityType, Args&&...)> func, Args&&... args)
		{
			for (size_t i = _entitySystem.SortEntities(); i != 0; i--)
			{
				entt::entity entity = _entitySystem.GetEntity(i - 1);
				func(entity, _entitySystem.GetType(entity), std::forward<Args>(args)...);
			}
		}

		ILevelService* _levelService;
		entt::registry _registry;
		entt::delegate<void(entt::entity, EntityType)> _advanceCallback;
		entt::delegate<void(entt::entity, EntityType, ff::PixelRendererActive&)> _renderCallback;

		EntitySystem _entitySystem;
		PositionSystem _positionSystem;
		CollisionSystem _collisionSystem;
		EntityFactory _entityFactory;
	};
}
