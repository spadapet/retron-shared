#pragma once

#include "Level/Entity.h"

namespace ReTron
{
	struct Player;

	class EntitySystem
	{
	public:
		EntitySystem(entt::registry& registry);

		entt::entity Create(EntityType type);
		EntityType GetType(entt::entity entity);
		void DelayDelete(entt::entity entity);
		void FlushDelete();

		void SetPlayer(entt::entity entity, size_t indexInLevel);
		size_t GetPlayer(entt::entity entity);
		entt::runtime_view GetPlayers();

	private:
		entt::registry& _registry;
	};
}
