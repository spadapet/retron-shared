#include "pch.h"
#include "Level/EntitySystem.h"

struct PendingDelete
{
};

struct PlayerComponent
{
	size_t _indexInLevel;
};

struct GruntComponent
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

	switch (type)
	{
	case EntityType::Grunt:
		_registry.emplace<GruntComponent>(entity);
		break;
	}

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
				return typeA < typeB;
			});
	}

	return GetEntityCount();
}

entt::sink<void(entt::entity)> ReTron::EntitySystem::EntityCreated()
{
	return _entityCreated;
}

entt::sink<void(entt::entity)> ReTron::EntitySystem::EntityDeleting()
{
	return _entityDeleting;
}

ReTron::EntityType ReTron::EntitySystem::GetType(entt::entity entity)
{
	return _registry.get<EntityType>(entity);
}

size_t ReTron::EntitySystem::GetEntityCount() const
{
	return _registry.size<EntityType>();
}

entt::entity ReTron::EntitySystem::GetEntity(size_t index) const
{
	return _registry.data<EntityType>()[index];
}

size_t ReTron::EntitySystem::GetGruntCount() const
{
	return _registry.size<GruntComponent>();
}

entt::entity ReTron::EntitySystem::GetGrunt(size_t index) const
{
	return _registry.data<GruntComponent>()[index];
}

void ReTron::EntitySystem::SetPlayer(entt::entity entity, size_t indexInLevel)
{
	_registry.emplace<PlayerComponent>(entity, indexInLevel);
}

size_t ReTron::EntitySystem::GetPlayer(entt::entity entity)
{
	PlayerComponent* pc = _registry.try_get<PlayerComponent>(entity);
	return pc ? pc->_indexInLevel : ff::INVALID_SIZE;
}
