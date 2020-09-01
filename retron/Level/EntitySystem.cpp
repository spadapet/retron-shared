#include "pch.h"
#include "Level/EntitySystem.h"

struct PendingDelete
{
};

ReTron::EntitySystem::EntitySystem(entt::registry& registry)
	: _registry(registry)
	, _sortEntities(false)
{
}

entt::entity ReTron::EntitySystem::Create(EntityType type)
{
	entt::entity entity = _registry.create();
	_registry.emplace<EntityType>(entity, type);
	_sortEntities = true;
	_entityCreated.publish(entity);

	return entity;
}

void ReTron::EntitySystem::DelayDelete(entt::entity entity)
{
	_registry.emplace_or_replace<PendingDelete>(entity);
	_entityDeleting.publish(entity);
}

void ReTron::EntitySystem::FlushDelete()
{
	for (entt::entity entity : _registry.view<PendingDelete>())
	{
		_registry.remove_all(entity);
	}
}

size_t ReTron::EntitySystem::SortEntities()
{
	if (_sortEntities)
	{
		_sortEntities = false;

		_registry.sort<EntityType>([](EntityType typeA, EntityType typeB)
			{
				// Since we loop backwards
				return typeA > typeB;
			});
	}

	return GetEntityCount();
}

size_t ReTron::EntitySystem::GetEntityCount() const
{
	return _registry.size<EntityType>();
}

entt::entity ReTron::EntitySystem::GetEntity(size_t index) const
{
	return _registry.data<EntityType>()[index];
}

ReTron::EntityType ReTron::EntitySystem::GetType(entt::entity entity)
{
	return _registry.get<EntityType>(entity);
}

entt::sink<void(entt::entity)> ReTron::EntitySystem::EntityCreated()
{
	return _entityCreated;
}

entt::sink<void(entt::entity)> ReTron::EntitySystem::EntityDeleting()
{
	return _entityDeleting;
}
