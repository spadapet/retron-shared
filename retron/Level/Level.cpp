#include "pch.h"
#include "Core/AppService.h"
#include "Core/GameService.h"
#include "Core/LevelService.h"
#include "Graph/Anim/Animation.h"
#include "Graph/Anim/Transform.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Graph/Sprite/Sprite.h"
#include "Graph/Texture/Palette.h"
#include "Input/InputMapping.h"
#include "Level/Level.h"
#include "Resource/Resources.h"

static ff::StaticString LEVEL_PARTICLES(L"LevelParticles");
static ff::StaticString PARTICLES_DestroyGrunt(L"DestroyGrunt");
static ff::StaticString PARTICLES_EnterPlayer(L"EnterPlayer");
static ff::StaticString PARTICLES_PlayerBulletHitBounds(L"PlayerBulletHitBounds");

struct PlayerData
{
	size_t _indexInLevel;
	size_t _shotCounter;
};

struct PlayerBulletData
{
	entt::entity _player;
};

struct GruntData
{
	size_t _index;
	size_t _moveCounter;
	ff::PointFixedInt _destPos;
};

struct ParticleEffectFollowsEntity
{
	int _effectId;
	ff::PointFixedInt _offset;
};

ReTron::Level::Level(ILevelService* levelService)
	: _levelService(levelService)
	, _gameService(levelService->GetGameService())
	, _appService(_gameService->GetAppService())
	, _gameSpec(_appService->GetGameSpec())
	, _levelSpec(levelService->GetLevelSpec())
	, _difficultySpec(_gameService->GetDifficultySpec())
	, _frames(0)
	, _entities(_registry)
	, _position(_registry)
	, _collision(_registry, _position, _entities)
{
	InitResources();

	_connections.emplace_front(_appService->GetReloadResourcesSink().connect<&Level::InitResources>(this));
	_advanceCallback.connect<&Level::AdvanceEntity>(this);
	_renderCallback.connect<&Level::RenderEntity>(this);

	for (size_t i = 0; i < _levelService->GetPlayerCount(); i++)
	{
		CreatePlayer(i);
	}

	std::vector<ff::RectFixedInt> avoidRects;
	avoidRects.reserve(_levelSpec._rects.size());

	for (const LevelRect& levelRect : _levelSpec._rects)
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

	for (const LevelObjectsSpec& objSpec : _levelSpec._objects)
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

	ff::AtScope particleScope = _particles.AdvanceAsync();
	EnumEntities(_advanceCallback);
	HandleCollisions();
	AdvanceParticleEffectPositions();
	_entities.FlushDelete();
}

void ReTron::Level::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect)
{
	ff::RendererActive renderActive = ff::PixelRendererActive::BeginRender(_appService->GetRenderer(), target, depth, targetRect, cameraRect);
	ff::PixelRendererActive pixelRender(renderActive);

	EnumEntities<ff::PixelRendererActive&>(_renderCallback, pixelRender);
	RenderParticles(pixelRender);
	RenderDebug(pixelRender);
}

void ReTron::Level::InitResources()
{
	_playerSprite.Init(L"sprites.player");
	_playerBulletSprite.Init(L"sprites.player-bullet");

	ff::AutoResourceValue levelParticlesRes(_appService->GetResources(), ::LEVEL_PARTICLES);
	ff::ValuePtrT<ff::DictValue> levelParticlesValue = levelParticlesRes.Flush();
	ff::Dict levelParticlesDict = levelParticlesValue.GetValue();

	_destroyGruntParticles = Particles::Effect(levelParticlesDict.GetValue(::PARTICLES_DestroyGrunt));
	_playerBulletHitBoundsParticles = Particles::Effect(levelParticlesDict.GetValue(::PARTICLES_PlayerBulletHitBounds));
	_playerEnterParticles = Particles::Effect(levelParticlesDict.GetValue(::PARTICLES_EnterPlayer));
}

entt::entity ReTron::Level::CreateEntity(EntityType type, const ff::PointFixedInt& pos)
{
	entt::entity entity = _entities.Create(type);
	_position.SetPosition(entity, pos);

	switch (type)
	{
	case EntityType::Grunt:
		_registry.emplace<GruntData>(entity, _registry.size<GruntData>(), PickGruntMoveCounter(), pos);
		break;
	}

	return entity;
}

entt::entity ReTron::Level::CreatePlayer(size_t indexInLevel)
{
	ff::PointFixedInt pos = _levelSpec._playerStart;
	pos.x += indexInLevel * 16 - _levelService->GetPlayerCount() * 8 + 8;

	entt::entity entity = CreateEntity(EntityType::Player, pos);
	_registry.emplace<PlayerData>(entity, PlayerData{ indexInLevel, 0 });

	ff::PointFixedInt effectOffset(0, -6);
	int effectId = _playerEnterParticles.Add(_particles, pos + effectOffset);
	_registry.emplace<ParticleEffectFollowsEntity>(entity, ParticleEffectFollowsEntity{ effectId, effectOffset });

	return entity;
}

entt::entity ReTron::Level::CreatePlayerBullet(entt::entity player, ff::PointFixedInt shotPos, ff::PointFixedInt shotDir)
{
	ff::PointFixedInt vel(_difficultySpec._playerShotMove * shotDir.x, _difficultySpec._playerShotMove * shotDir.y);
	entt::entity entity = CreateEntity(EntityType::PlayerBullet, shotPos + vel);
	_registry.emplace<PlayerBulletData>(entity, player);

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

	const ff::InputDevices& inputDevices = _gameService->GetInputDevices(player);
	const ff::IInputEvents* inputEvents = _gameService->GetInputEvents(player);

	ff::RectFixedInt dirPress(
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_LEFT),
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_UP),
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_RIGHT),
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_DOWN));

	dirPress.left = (dirPress.left >= _gameSpec._joystickMin) ? dirPress.left * _difficultySpec._playerMove : 0_f;
	dirPress.top = (dirPress.top >= _gameSpec._joystickMin) ? dirPress.top * _difficultySpec._playerMove : 0_f;
	dirPress.right = (dirPress.right >= _gameSpec._joystickMin) ? dirPress.right * _difficultySpec._playerMove : 0_f;
	dirPress.bottom = (dirPress.bottom >= _gameSpec._joystickMin) ? dirPress.bottom * _difficultySpec._playerMove : 0_f;

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

			CreatePlayerBullet(entity, shotPos, shotDir);
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
	if (!--data._moveCounter)
	{
		data._moveCounter = PickGruntMoveCounter();

		const size_t players = _registry.size<PlayerData>();
		entt::entity destEntity = players ? _registry.data<PlayerData>()[data._index % players] : entity;
		ff::PointFixedInt gruntPos = _position.GetPosition(entity);

		data._destPos = PickMoveDestination(entity, destEntity, CollisionBoxType::GruntAvoidBox);

		ff::PointFixedInt delta = data._destPos - gruntPos;
		ff::PointFixedInt vel(
			std::copysign(_difficultySpec._gruntMove, delta.x ? delta.x : Random::Sign()),
			std::copysign(_difficultySpec._gruntMove, delta.y ? delta.y : Random::Sign()));

		_position.SetPosition(entity, gruntPos + vel);
	}
}

void ReTron::Level::AdvanceParticleEffectPositions()
{
	for (auto [entity, data] : _registry.view<ParticleEffectFollowsEntity>().proxy())
	{
		if (_particles.IsEffectActive(data._effectId))
		{
			_particles.SetEffectPosition(data._effectId, _position.GetPosition(entity) + data._offset);
		}
		else
		{
			_registry.remove<ParticleEffectFollowsEntity>(entity);
		}
	}
}

void ReTron::Level::HandleCollisions()
{
	for (auto& [entity1, entity2] : _collision.DetectCollisions(_collisions, CollisionBoxType::BoundsBox))
	{
		HandleBoundsCollision(entity1, entity2);
	}

	for (auto& [entity1, entity2] : _collision.DetectCollisions(_collisions, CollisionBoxType::HitBox))
	{
		HandleEntityCollision(entity1, entity2);
	}
}

void ReTron::Level::HandleBoundsCollision(entt::entity entity1, entt::entity entity2)
{
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
	ff::PointFixedInt pos = _position.GetPosition(entity1) + offset;
	_position.SetPosition(entity1, pos);

	switch (_entities.GetType(entity1))
	{
	case EntityType::PlayerBullet:
		HandlePlayerBulletBoundsCollision(entity1);
		break;
	}
}

void ReTron::Level::HandlePlayerBulletBoundsCollision(entt::entity entity)
{
	_entities.DelayDelete(entity);

	ff::PointFixedInt vel = _position.GetVelocity(entity);
	ff::FixedInt angle = _position.GetReverseVelocityAsAngle(entity);
	ff::PointFixedInt pos = _position.GetPosition(entity);
	ff::PointFixedInt pos2(pos.x + (vel.x ? std::copysign(1_f, vel.x) : 0_f), pos.y + (vel.y ? std::copysign(1_f, vel.y) : 0_f));
	std::array<ff::PointFixedInt, 2> posArray{ pos, pos2 };

	Particles::EffectOptions options;
	options._angle = std::make_pair(angle - 60_f, angle + 60_f);

	// Set palette for bullet particles
	{
		PlayerBulletData& bulletData = _registry.get<PlayerBulletData>(entity);
		PlayerData* playerData = _registry.valid(bulletData._player) ? _registry.try_get<PlayerData>(bulletData._player) : nullptr;
		if (playerData)
		{
			Player& player = _levelService->GetPlayer(playerData->_indexInLevel);
			options._type = static_cast<uint8_t>(player._index);
		}
	}

	_playerBulletHitBoundsParticles.Add(_particles, posArray.data(), posArray.size(), &options);
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
			switch (_entities.GetType(entity1))
			{
			case EntityType::Grunt:
			{
				Particles::EffectOptions options;
				// options._angle
				_destroyGruntParticles.Add(_particles, _position.GetPosition(entity1), &options);
			}
			break;
			}

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

void ReTron::Level::RenderParticles(ff::PixelRendererActive& render)
{
	// Render particles for the default palette, which includes player 0
	_particles.Render(render, 0);

	// Render particles for other players
	for (size_t i = 0; i < _levelService->GetPlayerCount(); i++)
	{
		Player& player = _levelService->GetPlayer(i);
		if (player._index)
		{
			ff::IPalette* palette = _appService->GetPlayerPalette(player._index);
			render.GetRenderer()->PushPaletteRemap(palette->GetRemap(), palette->GetRemapHash());

			_particles.Render(render, static_cast<uint8_t>(player._index));

			render.GetRenderer()->PopPaletteRemap();
		}
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
	PlayerData& playerData = _registry.get<PlayerData>(entity);
	Player& player = _levelService->GetPlayer(playerData._indexInLevel);

	ff::IPalette* palette = _appService->GetPlayerPalette(player._index);
	render.GetRenderer()->PushPaletteRemap(palette->GetRemap(), palette->GetRemapHash());

	RenderAnimation(entity, render, _playerSprite.Flush(), 0);

	render.GetRenderer()->PopPaletteRemap();
}

void ReTron::Level::RenderPlayerBullet(entt::entity entity, ff::PixelRendererActive& render)
{
	RenderAnimation(entity, render, _playerBulletSprite.Flush(), 0);
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

void ReTron::Level::RenderAnimation(entt::entity entity, ff::PixelRendererActive& render, ff::IAnimation* anim, ff::FixedInt frame)
{
	ff::PointFloat pos = _position.GetPosition(entity).ToType<int>().ToType<float>();
	ff::PointFloat scale = _position.GetScale(entity).ToType<float>();
	ff::FixedInt rotation = _position.GetRotation(entity);

	anim->RenderFrame(render.GetRenderer(), ff::Transform::Create(pos, scale, rotation), frame);
}

void ReTron::Level::RenderDebug(ff::PixelRendererActive& render)
{
	if (_appService->ShouldRenderDebug())
	{
		for (auto [entity, data] : _registry.view<GruntData>().proxy())
		{
			render.DrawPaletteLine(_position.GetPosition(entity), data._destPos, 245, 1);
		}

		_collision.RenderDebug(render);
		_position.RenderDebug(render);
	}
}

size_t ReTron::Level::PickGruntMoveCounter()
{
	size_t i = std::min<size_t>(_frames / _difficultySpec._gruntMaxTicksRate, _difficultySpec._gruntMaxTicks - 1);
	i = Random::Range(1u, _difficultySpec._gruntMaxTicks - i) * _difficultySpec._gruntTickFrames;
	return std::max<size_t>(i, _difficultySpec._gruntMinTicks);
}

ff::PointFixedInt ReTron::Level::PickMoveDestination(entt::entity entity, entt::entity destEntity, CollisionBoxType collisionType)
{
	ff::PointFixedInt entityPos = _position.GetPosition(entity);
	ff::PointFixedInt destPos = _position.GetPosition(destEntity);
	ff::PointFixedInt result = destPos;

	auto [boxEntity, boxHitPos, boxHitNormal] = _collision.RayTest(entityPos, destPos, EntityBoxType::Level, collisionType);
	if (boxEntity != entt::null && boxHitPos != destPos)
	{
		ff::RectFixedInt box = _collision.GetBox(boxEntity, collisionType);
		ff::FixedInt bestDist = -1_f;
		for (ff::PointFixedInt corner : box.Corners())
		{
			ff::FixedInt dist = (corner - destPos).LengthSquared();
			if (bestDist < 0_f || dist < bestDist)
			{
				// Must be a clear path from the entity to the corner it chooses to move to
				auto [e2, p2, n2] = _collision.RayTest(entityPos, corner, EntityBoxType::Level, collisionType);
				if (e2 == entt::null || p2 == corner)
				{
					bestDist = dist;
					result = corner;
				}
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
