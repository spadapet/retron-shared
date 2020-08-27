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
		void DelayDelete(entt::entity entity);
		void FlushDelete();
		size_t SortEntities();

		entt::sink<void(entt::entity)> EntityCreated();
		entt::sink<void(entt::entity)> EntityDeleting();

		EntityType GetType(entt::entity entity);
		size_t GetEntityCount() const;
		entt::entity GetEntity(size_t index) const;

		size_t GetGruntCount() const;
		entt::entity GetGrunt(size_t index) const;

		void SetPlayer(entt::entity entity, size_t indexInLevel);
		size_t GetPlayer(entt::entity entity);

	private:
		entt::registry& _registry;
		entt::sigh<void(entt::entity)> _entityCreated;
		entt::sigh<void(entt::entity)> _entityDeleting;
		bool _sortEntities;
	};
}
