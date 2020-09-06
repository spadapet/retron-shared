#include "pch.h"
#include "Core/AppService.h"
#include "Core/GameService.h"
#include "Core/LevelService.h"
#include "Graph/Anim/Transform.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Graph/Sprite/Sprite.h"
#include "Input/InputMapping.h"
#include "Level/Level.h"

struct PlayerData
{
	size_t _indexInLevel;
	size_t _shotCounter;
};

struct GruntData
{
	size_t _index;
	size_t _moveCounter;
};

ReTron::Level::Level(ILevelService* levelService)
	: _levelService(levelService)
	, _difficultySpec(levelService->GetGameService()->GetDifficultySpec())
	, _entitySystem(_registry)
	, _positionSystem(_registry)
	, _collisionSystem(_registry, _positionSystem, _entitySystem)
	, _frames(0)
	, _playerBulletSprite(L"sprites.player-bullet")
{
	_advanceCallback.connect<&Level::AdvanceEntity>(this);
	_renderCallback.connect<&Level::RenderEntity>(this);

	for (size_t i = 0; i < _levelService->GetPlayerCount(); i++)
	{
		CreatePlayer(i);
	}

	const LevelSpec& spec = _levelService->GetLevelSpec();

	std::vector<ff::RectFixedInt> avoidRects;
	avoidRects.reserve(spec._rects.size());

	for (const LevelRect& levelRect : spec._rects)
	{
		switch (levelRect._type)
		{
		case LevelRect::Type::Bounds:
			CreateBounds(levelRect._rect.Deflate(Constants::LEVEL_BORDER_THICKNESS, Constants::LEVEL_BORDER_THICKNESS));
			break;

		case LevelRect::Type::Box:
			CreateBox(levelRect._rect);
			avoidRects.push_back(levelRect._rect);
			break;

		case LevelRect::Type::Safe:
			avoidRects.push_back(levelRect._rect);
			break;
		}
	}

	for (const LevelObjectsSpec& objSpec : spec._objects)
	{
		CreateObjects(objSpec._bonusWoman, EntityType::BonusWoman, objSpec._rect, avoidRects);
		CreateObjects(objSpec._bonusMan, EntityType::BonusMan, objSpec._rect, avoidRects);
		CreateObjects(objSpec._bonusChild, EntityType::BonusChild, objSpec._rect, avoidRects);
		CreateObjects(objSpec._electrode, EntityType::Electrode, objSpec._rect, avoidRects);
		CreateObjects(objSpec._hulk, EntityType::Hulk, objSpec._rect, avoidRects);
		CreateObjects(objSpec._grunt, EntityType::Grunt, objSpec._rect, avoidRects);
	}
}

ReTron::Level::~Level()
{
}

void ReTron::Level::Advance(ff::RectFixedInt cameraRect)
{
	_frames++;

	EnumEntities(_advanceCallback);
	HandleCollisions();

	_entitySystem.FlushDelete();
}

void ReTron::Level::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect)
{
	ff::RendererActive renderActive = ff::PixelRendererActive::BeginRender(_levelService->GetGameService()->GetAppService()->GetRenderer(), target, depth, targetRect, cameraRect);
	EnumEntities<ff::PixelRendererActive&>(_renderCallback, ff::PixelRendererActive(renderActive));

	if (_levelService->GetGameService()->GetAppService()->ShouldRenderDebug())
	{
		_collisionSystem.RenderDebug(ff::PixelRendererActive(renderActive));
		_positionSystem.RenderDebug(ff::PixelRendererActive(renderActive));
	}
}

entt::entity ReTron::Level::CreateEntity(EntityType type, const ff::PointFixedInt& pos)
{
	entt::entity entity = _entitySystem.Create(type);
	_positionSystem.SetPosition(entity, pos);

	switch (type)
	{
	case EntityType::Grunt:
		_registry.emplace<GruntData>(entity, _registry.size<GruntData>(), PickGruntMoveCounter());
		break;
	}

	return entity;
}

entt::entity ReTron::Level::CreatePlayer(size_t indexInLevel)
{
	ff::PointFixedInt pos = _levelService->GetLevelSpec()._playerStart;
	pos.x += indexInLevel * 16 - _levelService->GetPlayerCount() * 8 + 8;

	entt::entity entity = CreateEntity(EntityType::Player, pos);
	_registry.emplace<PlayerData>(entity, indexInLevel);

	return entity;
}

entt::entity ReTron::Level::CreatePlayerBullet(ff::PointFixedInt shotPos, ff::PointFixedInt shotDir)
{
	ff::PointFixedInt vel(_difficultySpec._playerShotMove * shotDir.x, _difficultySpec._playerShotMove * shotDir.y);
	entt::entity entity = CreateEntity(EntityType::PlayerBullet, shotPos + vel);

	_positionSystem.SetVelocity(entity, vel);
	_positionSystem.SetRotation(entity, Helpers::DirToDegrees(shotDir));

	return entity;
}

entt::entity ReTron::Level::CreateBounds(const ff::RectFixedInt& rect)
{
	entt::entity entity = _entitySystem.Create(EntityType::LevelBounds);
	_collisionSystem.SetBox(entity, rect, EntityBoxType::Level, CollisionBoxType::BoundsBox);
	return entity;
}

entt::entity ReTron::Level::CreateBox(const ff::RectFixedInt& rect)
{
	entt::entity entity = _entitySystem.Create(EntityType::LevelBox);
	_collisionSystem.SetBox(entity, rect, EntityBoxType::Level, CollisionBoxType::BoundsBox);
	return entity;
}

void ReTron::Level::CreateObjects(size_t count, EntityType type, const ff::RectFixedInt& rect, const std::vector<ff::RectFixedInt>& avoidRects)
{
	const ff::RectFixedInt& hitSpec = ReTron::GetHitBoxSpec(type);

	for (size_t i = 0; i < count; i++)
	{
		for (size_t attempt = 0; attempt < 2048; attempt++)
		{
			ff::PointFixedInt pos(
				(Random::Positive() & ~0x3) % (int)(rect.Width() - hitSpec.Width()) - (int)hitSpec.left + (int)rect.left,
				(Random::Positive() & ~0x3) % (int)(rect.Height() - hitSpec.Height()) - (int)hitSpec.top + (int)rect.top);

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

void ReTron::Level::AdvanceEntity(entt::entity entity, EntityType type)
{
	switch (type)
	{
	case EntityType::Player:
		AdvancePlayer(entity);
		break;

	case EntityType::PlayerBullet:
		AdvancePlayerBullet(entity);
		break;

	case EntityType::Grunt:
		AdvanceGrunt(entity);
		break;
	}
}

void ReTron::Level::AdvancePlayer(entt::entity entity)
{
	PlayerData& playerData = _registry.get<PlayerData>(entity);
	Player& player = _levelService->GetPlayer(playerData._indexInLevel);

	const ff::InputDevices& inputDevices = _levelService->GetGameService()->GetInputDevices(player);
	const ff::IInputEvents* inputEvents = _levelService->GetGameService()->GetInputEvents(player);
	const ff::FixedInt dirScale = 1.5_f;

	ff::RectFixedInt dirPress(
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_LEFT),
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_UP),
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_RIGHT),
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_DOWN));

	ff::PointFixedInt dir = _positionSystem.GetDirection(entity);
	dir.x = dirPress.left ? -1_f : (dirPress.right ? 1_f : dir.x);
	dir.y = dirPress.top ? -1_f : (dirPress.bottom ? 1_f : dir.y);
	_positionSystem.SetDirection(entity, dir);

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	pos = pos.Offset(dirPress.right - dirPress.left, dirPress.bottom - dirPress.top);
	_positionSystem.SetPosition(entity, pos);

	if (playerData._shotCounter)
	{
		playerData._shotCounter--;
	}

	if (!playerData._shotCounter)
	{
		ff::RectInt shotPress(
			inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_SHOOT_LEFT),
			inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_SHOOT_UP),
			inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_SHOOT_RIGHT),
			inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_SHOOT_DOWN));

		if (!shotPress.IsZeros())
		{
			playerData._shotCounter = _difficultySpec._playerShotCounter;

			ff::RectFixedInt box = _collisionSystem.GetBox(entity, CollisionBoxType::BoundsBox);
			ff::PointFixedInt shotPos = box.Center();
			ff::PointFixedInt shotDir(
				shotPress.left ? -1 : (shotPress.right ? 1 : 0),
				shotPress.top ? -1 : (shotPress.bottom ? 1 : 0));

			CreatePlayerBullet(shotPos, shotDir);
		}
	}
}

void ReTron::Level::AdvancePlayerBullet(entt::entity entity)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	ff::PointFixedInt vel = _positionSystem.GetVelocity(entity);
	_positionSystem.SetPosition(entity, pos + vel);
}

void ReTron::Level::AdvanceGrunt(entt::entity entity)
{
	GruntData& data = _registry.get<GruntData>(entity);
	if (--data._moveCounter)
	{
		return;
	}

	data._moveCounter = PickGruntMoveCounter();

	// Must be a player to target
	if (!_registry.size<PlayerData>())
	{
		return;
	}

	// Spread the grunts out to target each player evenly
	size_t player = data._index % _registry.size<PlayerData>();
	entt::entity playerEntity = _registry.data<PlayerData>()[player];
	ff::PointFixedInt playerPos = _positionSystem.GetPosition(playerEntity);
	ff::PointFixedInt gruntPos = _positionSystem.GetPosition(entity);
	ff::PointFixedInt destPos = playerPos;

	// Move around boxes in the level
	auto [boxEntity, boxHitPos, boxHitNormal] = _collisionSystem.RayTest(gruntPos, playerPos, CollisionBoxType::BoundsBox);
	if (boxEntity != entt::null && boxHitPos != playerPos)
	{
		ff::RectFixedInt entityBoxSpec = _collisionSystem.GetBoxSpec(entity, CollisionBoxType::BoundsBox);
		ff::RectFixedInt box = _collisionSystem.GetBox(boxEntity, CollisionBoxType::BoundsBox);
		box = box.Inflate(entityBoxSpec.right, entityBoxSpec.bottom, -entityBoxSpec.left, -entityBoxSpec.top);

		ff::FixedInt bestDist = -1_f;
		for (ff::PointFixedInt corner : box.Corners())
		{
			ff::FixedInt dist = (corner - playerPos).LengthSquared();
			if (bestDist < 0_f || dist < bestDist)
			{
				bestDist = dist;
				destPos = corner;
			}
		}
	}

	gruntPos.x += (gruntPos.x != destPos.x)
		? std::copysign(_difficultySpec._gruntMove, destPos.x - gruntPos.x)
		: std::copysign(_difficultySpec._gruntMove, ff::FixedInt(Random::Sign())); // always move diagnally

	gruntPos.y += (gruntPos.y != destPos.y)
		? std::copysign(_difficultySpec._gruntMove, destPos.y - gruntPos.y)
		: std::copysign(_difficultySpec._gruntMove, ff::FixedInt(Random::Sign())); // always move diagnally

	_positionSystem.SetPosition(entity, gruntPos);
}

void ReTron::Level::HandleCollisions()
{
	bool boundsCollision = true;
	for (size_t pass = 0; boundsCollision && pass != 2; pass++)
	{
		boundsCollision = false;
		for (size_t i = _collisionSystem.DetectCollisions(CollisionBoxType::BoundsBox); i != 0; i--)
		{
			auto [entity1, entity2] = _collisionSystem.GetCollision(i - 1);
			HandleBoundsCollision(entity1, entity2);
			boundsCollision = true;
		}
	}

	for (size_t i = _collisionSystem.DetectCollisions(CollisionBoxType::HitBox); i != 0; i--)
	{
		auto [entity1, entity2] = _collisionSystem.GetCollision(i - 1);
		HandleEntityCollision(entity1, entity2);
	}
}

void ReTron::Level::HandleBoundsCollision(entt::entity entity1, entt::entity entity2)
{
	EntityType entityType = _entitySystem.GetType(entity1);
	switch (entityType)
	{
	case EntityType::PlayerBullet:
		_entitySystem.DelayDelete(entity1);
		break;
	}

	ff::RectFixedInt oldRect = _collisionSystem.GetBox(entity1, CollisionBoxType::BoundsBox);
	ff::RectFixedInt newRect = oldRect;

	ff::RectFixedInt levelRect = _collisionSystem.GetBox(entity2, CollisionBoxType::BoundsBox);
	EntityType levelType = _entitySystem.GetType(entity2);

	switch (levelType)
	{
	case EntityType::LevelBounds:
		newRect = oldRect.MoveInside(levelRect);
		break;

	case EntityType::LevelBox:
		newRect = oldRect.MoveOutside(levelRect);
		break;
	}

	ff::PointFixedInt offset = newRect.TopLeft() - oldRect.TopLeft();
	_positionSystem.SetPosition(entity1, _positionSystem.GetPosition(entity1) + offset);
}

void ReTron::Level::HandleEntityCollision(entt::entity entity1, entt::entity entity2)
{
	EntityBoxType type1 = _collisionSystem.GetBoxType(entity1, CollisionBoxType::HitBox);
	EntityBoxType type2 = _collisionSystem.GetBoxType(entity2, CollisionBoxType::HitBox);

	switch (type1)
	{
	case EntityBoxType::Enemy:
		switch (type2)
		{
		case EntityBoxType::PlayerBullet:
			if (_entitySystem.DelayDelete(entity2))
			{
				_entitySystem.DelayDelete(entity1);
			}
			break;
		}
		break;

	case EntityBoxType::PlayerBullet:
		switch (type2)
		{
		case EntityBoxType::EnemyBullet:
			if (_entitySystem.DelayDelete(entity1))
			{
				_entitySystem.DelayDelete(entity2);
			}
			break;
		}
		break;
	}
}

void ReTron::Level::RenderEntity(entt::entity entity, EntityType type, ff::PixelRendererActive& render)
{
	switch (type)
	{
	case EntityType::Player:
		RenderPlayer(entity, render);
		break;

	case EntityType::PlayerBullet:
		RenderPlayerBullet(entity, render);
		break;

	case EntityType::BonusWoman:
	case EntityType::BonusMan:
	case EntityType::BonusChild:
		RenderBonus(entity, type, render);
		break;

	case EntityType::Electrode:
		RenderElectrode(entity, render);
		break;

	case EntityType::Hulk:
		RenderHulk(entity, render);
		break;

	case EntityType::Grunt:
		RenderGrunt(entity, render);
		break;

	case EntityType::LevelBounds:
	case EntityType::LevelBox:
		render.DrawPaletteOutlineRectangle(_collisionSystem.GetBox(entity, CollisionBoxType::BoundsBox), Colors::LEVEL_BORDER,
			(type == EntityType::LevelBounds) ? -Constants::LEVEL_BORDER_THICKNESS : Constants::LEVEL_BOX_THICKNESS);
		break;
	}

}

void ReTron::Level::RenderPlayer(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledCircle(pos.Offset(0, -4), 4, 236);
}

void ReTron::Level::RenderPlayerBullet(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	ff::PointFixedInt scale = _positionSystem.GetScale(entity);
	ff::FixedInt rotation = _positionSystem.GetRotation(entity);

	render.DrawSprite(_playerBulletSprite.Flush(), ff::PixelTransform::Create(pos, scale, rotation));
}

void ReTron::Level::RenderBonus(entt::entity entity, EntityType type, ff::PixelRendererActive& render)
{
	int color = 251;
	switch (type)
	{
	case EntityType::BonusWoman: color = 238; break;
	case EntityType::BonusMan: color = 246; break;
	}

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetBox(entity, CollisionBoxType::HitBox), color);
}

void ReTron::Level::RenderElectrode(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetBox(entity, CollisionBoxType::HitBox), 45);
}

void ReTron::Level::RenderHulk(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetBox(entity, CollisionBoxType::HitBox), 235);
}

void ReTron::Level::RenderGrunt(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetBox(entity, CollisionBoxType::HitBox), 248);
}

size_t ReTron::Level::PickGruntMoveCounter()
{
	size_t i = std::min<size_t>(_frames / _difficultySpec._gruntMaxTicksRate, _difficultySpec._gruntMaxTicks - 1);
	i = Random::RangeSize(1, _difficultySpec._gruntMaxTicks - i) * _difficultySpec._gruntTickFrames;
	return std::max<size_t>(i, _difficultySpec._gruntMinTicks);
}

template<typename... Args>
void ReTron::Level::EnumEntities(entt::delegate<void(entt::entity, EntityType, Args&&...)> func, Args&&... args)
{
	for (size_t i = _entitySystem.SortEntities(); i != 0; i--)
	{
		entt::entity entity = _entitySystem.GetEntity(i - 1);
		func(entity, _entitySystem.GetType(entity), std::forward<Args>(args)...);
	}
}
