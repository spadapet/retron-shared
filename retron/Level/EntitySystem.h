#pragma once

#include "Level/Entity.h"

namespace ReTron
{
	class EntitySystem
	{
	public:
		EntitySystem(entt::registry& registry);

		entt::entity Create(EntityType type);
		void DelayDelete(entt::entity entity);
		void FlushDelete();

		size_t SortEntities();
		size_t GetEntityCount() const;
		entt::entity GetEntity(size_t index) const;
		EntityType GetType(entt::entity entity);

		entt::sink<void(entt::entity)> EntityCreated();
		entt::sink<void(entt::entity)> EntityDeleting();

	private:
		entt::registry& _registry;
		entt::sigh<void(entt::entity)> _entityCreated;
		entt::sigh<void(entt::entity)> _entityDeleting;
		bool _sortEntities;
	};
}
