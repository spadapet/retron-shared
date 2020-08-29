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

entt::entity ReTron::EntityFactory::CreateEntity(EntityType type, const ff::PointFixedInt& pos)
{
	entt::entity entity = _entitySystem.Create(type);
	_positionSystem.SetPosition(entity, pos);
	return entity;
}

entt::entity ReTron::EntityFactory::CreatePlayer(size_t indexInLevel)
{
	Player& player = _levelService->GetPlayer(indexInLevel);
	ff::PointFixedInt pos = _levelService->GetLevelSpec()._playerStart;
	pos.x += indexInLevel * 16;
	pos.x -= _levelService->GetPlayerCount() * 8 - 8;

	entt::entity entity = CreateEntity(EntityType::Player, pos);
	_entitySystem.SetPlayer(entity, indexInLevel);

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

void ReTron::EntityFactory::CreateObjects(size_t count, EntityType type, const ff::RectFixedInt& rect, const std::vector<ff::RectFixedInt>& avoidRects)
{
	const ff::RectFixedInt& hitSpec = ReTron::GetHitBoxSpec(type);

	for (size_t i = 0; i < count; i++)
	{
		for (size_t attempt = 0; attempt < 2048; attempt++)
		{
			ff::PointFixedInt pos(
				(std::rand() << 2) % (int)(rect.Width() - hitSpec.Width()) - (int)hitSpec.left + (int)rect.left,
				(std::rand() << 2) % (int)(rect.Height() - hitSpec.Height()) - (int)hitSpec.top + (int)rect.top);

			ff::RectFixedInt hitRect = hitSpec + pos;
			bool goodPos = true;

			for (const ff::RectFixedInt& avoidRect : avoidRects)
			{
				if (hitRect.DoesIntersect(avoidRect))
				{
					goodPos = false;
					break;
				}
			}

			if (goodPos)
			{
				CreateEntity(type, pos);
				break;
			}
		}
	}
}
