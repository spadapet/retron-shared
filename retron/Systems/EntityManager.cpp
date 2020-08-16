#include "pch.h"
#include "Systems/EntityManager.h"

struct PendingDelete
{
};

ReTron::EntityManager::EntityManager(entt::registry& registry)
	: _registry(registry)
{
}

void ReTron::EntityManager::FlushDelete()
{
	for (entt::entity entity : _registry.view<PendingDelete>())
	{
		_registry.remove_all(entity);
	}
}

void ReTron::EntityManager::DelayDelete(entt::entity entity)
{
	_registry.emplace_or_replace<PendingDelete>(entity);
}

entt::entity ReTron::EntityManager::CreatePlayer(size_t index)
{
	return _registry.create();
}
