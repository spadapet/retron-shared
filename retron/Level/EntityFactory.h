#pragma once

namespace ReTron
{
	class CollisionSystem;;
	class EntitySystem;
	class ILevelService;
	class PositionSystem;

	class EntityFactory
	{
	public:
		EntityFactory(ILevelService* levelService, entt::registry& registry, EntitySystem& entitySystem, PositionSystem& positionSystem, CollisionSystem& collisionSystem);

		entt::entity CreatePlayer(size_t indexInLevel);
		entt::entity CreateBounds(const ff::RectFixedInt& rect);
		entt::entity CreateBox(const ff::RectFixedInt& rect);

	private:
		ILevelService* _levelService;
		EntitySystem& _entitySystem;
		PositionSystem& _positionSystem;
		CollisionSystem& _collisionSystem;
		entt::registry& _registry;
	};
}
