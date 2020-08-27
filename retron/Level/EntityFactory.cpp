#include "pch.h"
#include "Core/GameSpec.h"
#include "Core/LevelService.h"
#include "Core/Player.h"
#include "Level/CollisionSystem.h"
#include "Level/EntityFactory.h"
#include "Level/EntitySystem.h"
#include "Level/PositionSystem.h"

ReTron::EntityFactory::EntityFactory(ILevelService* levelService, entt::registry& registry, EntitySystem& entitySystem, PositionSystem& positionSystem, CollisionSystem& collisionSystem)
	: _levelService(levelService)
	, _entitySystem(entitySystem)
	, _positionSystem(positionSystem)
	, _collisionSystem(collisionSystem)
	, _registry(registry)
{
}

entt::entity ReTron::EntityFactory::CreatePlayer(size_t indexInLevel)
{
	Player& player = _levelService->GetPlayer(indexInLevel);
	ff::PointFixedInt pos = _levelService->GetLevelSpec()._playerStart;

	pos.x += indexInLevel * 16;
	pos.x -= _levelService->GetPlayerCount() * 8 - 8;

	entt::entity entity = _entitySystem.Create(EntityType::Player);
	_entitySystem.SetPlayer(entity, indexInLevel);
	_positionSystem.SetPosition(entity, pos);

	return entity;
}

entt::entity ReTron::EntityFactory::CreateBounds(const ff::RectFixedInt& rect)
{
	entt::entity entity = _entitySystem.Create(EntityType::LevelBounds);
	_collisionSystem.SetHitBox(entity, rect, EntityHitBoxType::Level);
	return entity;
}

entt::entity ReTron::EntityFactory::CreateBox(const ff::RectFixedInt& rect)
{
	entt::entity entity = _entitySystem.Create(EntityType::LevelBox);
	_collisionSystem.SetHitBox(entity, rect, EntityHitBoxType::Level);
	return entity;
}
