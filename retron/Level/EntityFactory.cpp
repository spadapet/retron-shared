#include "pch.h"
#include "Core/GameSpec.h"
#include "Core/Player.h"
#include "Level/EntityFactory.h"
#include "Level/EntitySystem.h"
#include "Level/PositionSystem.h"
#include "Services/LevelService.h"

ReTron::EntityFactory::EntityFactory(ILevelService* levelService, entt::registry& registry, EntitySystem& entitySystem, PositionSystem& positionSystem)
	: _levelService(levelService)
	, _entitySystem(entitySystem)
	, _positionSystem(positionSystem)
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
