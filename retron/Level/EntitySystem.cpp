#include "pch.h"
#include "Level/EntitySystem.h"

struct PendingDelete
{
};

struct PlayerComponent
{
	size_t _indexInLevel;
};

ReTron::EntitySystem::EntitySystem(entt::registry& registry)
	: _registry(registry)
{
}

entt::entity ReTron::EntitySystem::Create(EntityType type)
{
	entt::entity entity = _registry.create();
	_registry.emplace<EntityType>(entity, type);
	return entity;
}

ReTron::EntityType ReTron::EntitySystem::GetType(entt::entity entity)
{
	return _registry.get<EntityType>(entity);
}

void ReTron::EntitySystem::DelayDelete(entt::entity entity)
{
	_registry.emplace_or_replace<PendingDelete>(entity);
}

void ReTron::EntitySystem::FlushDelete()
{
	for (entt::entity entity : _registry.view<PendingDelete>())
	{
		_registry.remove_all(entity);
	}
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

entt::runtime_view ReTron::EntitySystem::GetPlayers()
{
	entt::id_type type = entt::type_info<PlayerComponent>::id();
	return _registry.runtime_view(&type, &type + 1);
}
