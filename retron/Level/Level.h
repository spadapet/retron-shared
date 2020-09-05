#pragma once

#include "Core/GameSpec.h"
#include "Level/CollisionSystem.h"
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
		entt::entity CreateEntity(EntityType type, const ff::PointFixedInt& pos);
		entt::entity CreatePlayer(size_t indexInLevel);
		entt::entity CreateBounds(const ff::RectFixedInt& rect);
		entt::entity CreateBox(const ff::RectFixedInt& rect);
		void CreateObjects(size_t count, EntityType type, const ff::RectFixedInt& rect, const std::vector<ff::RectFixedInt>& avoidRects);

		void AdvanceEntity(entt::entity entity, EntityType type);
		void AdvancePlayer(entt::entity entity);
		void AdvanceGrunt(entt::entity entity);

		void HandleCollisions();
		void HandleBoundsCollision(entt::entity entity1, entt::entity entity2);
		void HandleEntityCollision(entt::entity entity1, entt::entity entity2);

		void RenderEntity(entt::entity entity, EntityType type, ff::PixelRendererActive& render);
		void RenderPlayer(entt::entity entity, ff::PixelRendererActive& render);
		void RenderBonus(entt::entity entity, EntityType type, ff::PixelRendererActive& render);
		void RenderElectrode(entt::entity entity, ff::PixelRendererActive& render);
		void RenderHulk(entt::entity entity, ff::PixelRendererActive& render);
		void RenderGrunt(entt::entity entity, ff::PixelRendererActive& render);

		size_t PickGruntMoveCounter();

		template<typename... Args>
		void EnumEntities(entt::delegate<void(entt::entity, EntityType, Args&&...)> func, Args&&... args);

		ILevelService* _levelService;
		const DifficultySpec& _difficultySpec;
		size_t _frames;

		entt::registry _registry;
		entt::delegate<void(entt::entity, EntityType)> _advanceCallback;
		entt::delegate<void(entt::entity, EntityType, ff::PixelRendererActive&)> _renderCallback;
		std::forward_list<entt::scoped_connection> _connections;

		EntitySystem _entitySystem;
		PositionSystem _positionSystem;
		CollisionSystem _collisionSystem;
	};
}
