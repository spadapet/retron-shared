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
	, _entities(_registry)
	, _position(_registry)
	, _collision(_registry, _position, _entities)
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

	_entities.FlushDelete();
}

void ReTron::Level::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect)
{
	ff::RendererActive renderActive = ff::PixelRendererActive::BeginRender(_levelService->GetGameService()->GetAppService()->GetRenderer(), target, depth, targetRect, cameraRect);
	EnumEntities<ff::PixelRendererActive&>(_renderCallback, ff::PixelRendererActive(renderActive));

	if (_levelService->GetGameService()->GetAppService()->ShouldRenderDebug())
	{
		_collision.RenderDebug(ff::PixelRendererActive(renderActive));
		_position.RenderDebug(ff::PixelRendererActive(renderActive));
	}
}

entt::entity ReTron::Level::CreateEntity(EntityType type, const ff::PointFixedInt& pos)
{
	entt::entity entity = _entities.Create(type);
	_position.SetPosition(entity, pos);

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

	_position.SetVelocity(entity, vel);
	_position.SetRotation(entity, Helpers::DirToDegrees(shotDir));

	return entity;
}

entt::entity ReTron::Level::CreateBounds(const ff::RectFixedInt& rect)
{
	entt::entity entity = _entities.Create(EntityType::LevelBounds);
	_collision.SetBox(entity, rect, EntityBoxType::Level, CollisionBoxType::BoundsBox);
	return entity;
}

entt::entity ReTron::Level::CreateBox(const ff::RectFixedInt& rect)
{
	entt::entity entity = _entities.Create(EntityType::LevelBox);
	_collision.SetBox(entity, rect, EntityBoxType::Level, CollisionBoxType::BoundsBox);
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

	ff::PointFixedInt dir = _position.GetDirection(entity);
	dir.x = dirPress.left ? -1_f : (dirPress.right ? 1_f : dir.x);
	dir.y = dirPress.top ? -1_f : (dirPress.bottom ? 1_f : dir.y);
	_position.SetDirection(entity, dir);

	ff::PointFixedInt pos = _position.GetPosition(entity);
	pos = pos.Offset(dirPress.right - dirPress.left, dirPress.bottom - dirPress.top);
	_position.SetPosition(entity, pos);

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

			ff::RectFixedInt box = _collision.GetBox(entity, CollisionBoxType::BoundsBox);
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
	ff::PointFixedInt pos = _position.GetPosition(entity);
	ff::PointFixedInt vel = _position.GetVelocity(entity);
	_position.SetPosition(entity, pos + vel);
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

	entt::entity playerEntity = _registry.data<PlayerData>()[data._index % _registry.size<PlayerData>()];
	ff::PointFixedInt destPos = PickMoveDestination(entity, playerEntity);
	ff::PointFixedInt gruntPos = _position.GetPosition(entity);

	ff::PointFixedInt vel(
		(gruntPos.x != destPos.x)
			? std::copysign(_difficultySpec._gruntMove, destPos.x - gruntPos.x)
			: std::copysign(_difficultySpec._gruntMove, ff::FixedInt(Random::Sign())),
		(gruntPos.y != destPos.y)
			? std::copysign(_difficultySpec._gruntMove, destPos.y - gruntPos.y)
			: std::copysign(_difficultySpec._gruntMove, ff::FixedInt(Random::Sign())));

	// Adjust movement if colliding into a wall
	//{
	//	ff::RectFixedInt gruntBox = _collision.GetBox(entity, CollisionBoxType::BoundsBox);
	//	ff::RectFixedInt testBox = gruntBox + vel;

	//	for (entt::entity hitEntity : _collision.HitTest(testBox, _hits, EntityBoxType::Level, CollisionBoxType::BoundsBox))
	//	{
	//		ff::RectFixedInt levelBox = _collision.GetBox(hitEntity, CollisionBoxType::BoundsBox);
	//		if (levelBox.LeftEdge().DoesIntersect(testBox) || levelBox.RightEdge().DoesIntersect(testBox))
	//		{
	//			vel.y = (data._index % 2) ? _difficultySpec._gruntMove : -_difficultySpec._gruntMove;
	//		}
	//		else if (levelBox.TopEdge().DoesIntersect(testBox) || levelBox.BottomEdge().DoesIntersect(testBox))
	//		{
	//			vel.x = (data._index % 2) ? _difficultySpec._gruntMove : -_difficultySpec._gruntMove;
	//		}
	//	}
	//}

	_position.SetPosition(entity, gruntPos + vel);
}

void ReTron::Level::HandleCollisions()
{
	bool boundsCollision = true;
	for (size_t pass = 0; boundsCollision && pass != 2; pass++)
	{
		boundsCollision = false;
		for (size_t i = _collision.DetectCollisions(CollisionBoxType::BoundsBox); i != 0; i--)
		{
			auto [entity1, entity2] = _collision.GetCollision(i - 1);
			HandleBoundsCollision(entity1, entity2);
			boundsCollision = true;
		}
	}

	for (size_t i = _collision.DetectCollisions(CollisionBoxType::HitBox); i != 0; i--)
	{
		auto [entity1, entity2] = _collision.GetCollision(i - 1);
		HandleEntityCollision(entity1, entity2);
	}
}

void ReTron::Level::HandleBoundsCollision(entt::entity entity1, entt::entity entity2)
{
	EntityType entityType = _entities.GetType(entity1);
	switch (entityType)
	{
	case EntityType::PlayerBullet:
		_entities.DelayDelete(entity1);
		break;
	}

	ff::RectFixedInt oldRect = _collision.GetBox(entity1, CollisionBoxType::BoundsBox);
	ff::RectFixedInt newRect = oldRect;

	ff::RectFixedInt levelRect = _collision.GetBox(entity2, CollisionBoxType::BoundsBox);
	EntityType levelType = _entities.GetType(entity2);

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
	_position.SetPosition(entity1, _position.GetPosition(entity1) + offset);
}

void ReTron::Level::HandleEntityCollision(entt::entity entity1, entt::entity entity2)
{
	EntityBoxType type1 = _collision.GetBoxType(entity1, CollisionBoxType::HitBox);
	EntityBoxType type2 = _collision.GetBoxType(entity2, CollisionBoxType::HitBox);

	switch (type1)
	{
	case EntityBoxType::Enemy:
		switch (type2)
		{
		case EntityBoxType::PlayerBullet:
			if (_entities.DelayDelete(entity2))
			{
				_entities.DelayDelete(entity1);
			}
			break;
		}
		break;

	case EntityBoxType::PlayerBullet:
		switch (type2)
		{
		case EntityBoxType::EnemyBullet:
			if (_entities.DelayDelete(entity1))
			{
				_entities.DelayDelete(entity2);
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
		render.DrawPaletteOutlineRectangle(_collision.GetBox(entity, CollisionBoxType::BoundsBox), Colors::LEVEL_BORDER,
			(type == EntityType::LevelBounds) ? -Constants::LEVEL_BORDER_THICKNESS : Constants::LEVEL_BOX_THICKNESS);
		break;
	}

}

void ReTron::Level::RenderPlayer(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _position.GetPosition(entity);
	render.DrawPaletteFilledCircle(pos.Offset(0, -4), 4, 236);
}

void ReTron::Level::RenderPlayerBullet(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _position.GetPosition(entity);
	ff::PointFixedInt scale = _position.GetScale(entity);
	ff::FixedInt rotation = _position.GetRotation(entity);

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

	ff::PointFixedInt pos = _position.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collision.GetBox(entity, CollisionBoxType::HitBox), color);
}

void ReTron::Level::RenderElectrode(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _position.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collision.GetBox(entity, CollisionBoxType::HitBox), 45);
}

void ReTron::Level::RenderHulk(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _position.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collision.GetBox(entity, CollisionBoxType::HitBox), 235);
}

void ReTron::Level::RenderGrunt(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _position.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collision.GetBox(entity, CollisionBoxType::HitBox), 248);
}

size_t ReTron::Level::PickGruntMoveCounter()
{
	size_t i = std::min<size_t>(_frames / _difficultySpec._gruntMaxTicksRate, _difficultySpec._gruntMaxTicks - 1);
	i = Random::RangeSize(1, _difficultySpec._gruntMaxTicks - i) * _difficultySpec._gruntTickFrames;
	return std::max<size_t>(i, _difficultySpec._gruntMinTicks);
}

ff::PointFixedInt ReTron::Level::PickMoveDestination(entt::entity entity, entt::entity destEntity)
{
	ff::PointFixedInt entityPos = _position.GetPosition(entity);
	ff::PointFixedInt destPos = _position.GetPosition(destEntity);
	ff::PointFixedInt result = destPos;

	// Move around boxes in the level
	auto [boxEntity, boxHitPos, boxHitNormal] = _collision.RayTest(entityPos, destPos, CollisionBoxType::BoundsBox);
	if (boxEntity != entt::null && boxHitPos != destPos)
	{
		ff::RectFixedInt entityBoxSpec = _collision.GetBoxSpec(entity, CollisionBoxType::BoundsBox);
		ff::RectFixedInt box = _collision.GetBox(boxEntity, CollisionBoxType::BoundsBox);
		box = box.Inflate(entityBoxSpec.right, entityBoxSpec.bottom, -entityBoxSpec.left, -entityBoxSpec.top);

		ff::FixedInt bestDist = -1_f;
		for (ff::PointFixedInt corner : box.Corners())
		{
			ff::FixedInt dist = (corner - destPos).LengthSquared();
			if (bestDist < 0_f || dist < bestDist)
			{
				bestDist = dist;
				result = corner;
			}
		}
	}

	return result;
}

template<typename... Args>
void ReTron::Level::EnumEntities(entt::delegate<void(entt::entity, EntityType, Args&&...)> func, Args&&... args)
{
	for (size_t i = _entities.SortEntities(); i != 0; i--)
	{
		entt::entity entity = _entities.GetEntity(i - 1);
		func(entity, _entities.GetType(entity), std::forward<Args>(args)...);
	}
}
