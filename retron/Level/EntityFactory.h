#pragma once

namespace ReTron
{
	class EntitySystem;
	class ILevelService;
	class PositionSystem;

	class EntityFactory
	{
	public:
		EntityFactory(ILevelService* levelService, entt::registry& registry, EntitySystem& entitySystem, PositionSystem& positionSystem);

		entt::entity CreatePlayer(size_t indexInLevel);

	private:
		ILevelService* _levelService;
		EntitySystem& _entitySystem;
		PositionSystem& _positionSystem;
		entt::registry& _registry;
	};
}
