#include "pch.h"
#include "Systems/CollisionSystem.h"
#include "Systems/PositionSystem.h"

static const ff::FixedInt PIXEL_TO_WORLD_SCALE = 0.0625_f;
static const ff::FixedInt WORLD_TO_PIXEL_SCALE = 16_f;

struct HitBoxSpecComponent
{
	ff::PointFixedInt _topLeft;
	ff::PointFixedInt _size;
	ReTron::HitBoxType _type;
};

struct HitBoxComponent
{
	b2Body* _body;
	b2Fixture* _fixture;
};

struct DirtyComponent
{
};

static entt::entity EntityFromUserData(void* data)
{
	return static_cast<entt::entity>(reinterpret_cast<size_t>(data));
}

static void* UserDataFromEntity(entt::entity entity)
{
	return reinterpret_cast<void*>(static_cast<size_t>(entity));
}

ReTron::CollisionSystem::CollisionSystem(entt::registry& registry, PositionSystem& positionSystem)
	: _positionSystem(positionSystem)
	, _registry(registry)
	, _world(b2Vec2(0, 0))
{
	_world.SetAllowSleeping(false);
	_world.SetContactFilter(this);

	_connections.emplace_front(_registry.on_destroy<HitBoxComponent>().connect<&CollisionSystem::OnHitBoxRemoved>(this));
	_connections.emplace_front(_registry.on_construct<HitBoxSpecComponent>().connect<&CollisionSystem::OnHitBoxSpecChanged>(this));
	_connections.emplace_front(_registry.on_update<HitBoxSpecComponent>().connect<&CollisionSystem::OnHitBoxSpecChanged>(this));
	_connections.emplace_front(positionSystem.PositionChanged().connect<&CollisionSystem::OnPositionChanged>(this));
	_connections.emplace_front(positionSystem.RotationChanged().connect<&CollisionSystem::OnPositionChanged>(this));
	_connections.emplace_front(positionSystem.DirectionChanged().connect<&CollisionSystem::OnScaleChanged>(this));
	_connections.emplace_front(positionSystem.ScaleChanged().connect<&CollisionSystem::OnScaleChanged>(this));
}

void ReTron::CollisionSystem::DetectCollisions(std::vector<std::pair<entt::entity, entt::entity>>& pairs)
{
	UpdateDirtyHitBoxes();

	_world.Step(ff::SECONDS_PER_ADVANCE_F, 1, 1);

	for (const b2Contact* i = _world.GetContactList(); i; i = i->GetNext())
	{
		entt::entity entityA = ::EntityFromUserData(i->GetFixtureA()->GetUserData());
		entt::entity entityB = ::EntityFromUserData(i->GetFixtureB()->GetUserData());

		const HitBoxSpecComponent& specA = _registry.get<HitBoxSpecComponent>(entityA);
		const HitBoxSpecComponent& specB = _registry.get<HitBoxSpecComponent>(entityB);

		if (specA._type < specB._type)
		{
			pairs.emplace_back(entityA, entityB);
		}
		else
		{
			pairs.emplace_back(entityB, entityA);
		}
	}
}

void ReTron::CollisionSystem::HitTest(ff::RectFixedInt bounds, std::vector<entt::entity>& entities)
{
	UpdateDirtyHitBoxes();

	class Callback : public b2QueryCallback
	{
	public:
		Callback(std::vector<entt::entity>& entities)
			: _entities(entities)
		{
		}

		virtual bool ReportFixture(b2Fixture* fixture) override
		{
			_entities.push_back(::EntityFromUserData(fixture->GetUserData()));
			return true;
		}

	private:
		std::vector<entt::entity>& _entities;
	} callback(entities);

	bounds = bounds * ::PIXEL_TO_WORLD_SCALE;

	b2AABB aabb;
	aabb.lowerBound.Set(bounds.left, bounds.top);
	aabb.upperBound.Set(bounds.right, bounds.bottom);

	_world.QueryAABB(&callback, aabb);
}

void ReTron::CollisionSystem::SetHitBox(entt::entity entity, const ff::PointFixedInt& topLeft, const ff::PointFixedInt& size, HitBoxType type)
{
	_registry.emplace_or_replace<HitBoxSpecComponent>(entity, topLeft, size, type);
}

ff::RectFixedInt ReTron::CollisionSystem::GetHitBox(entt::entity entity)
{
	if (UpdateHitBox(entity))
	{
		const HitBoxComponent& hb = _registry.get<HitBoxComponent>(entity);
		b2AABB aabb;
		hb._fixture->GetShape()->ComputeAABB(&aabb, hb._body->GetTransform(), 0);

		return ff::RectFixedInt(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y) * ::WORLD_TO_PIXEL_SCALE;
	}

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	return ff::RectFixedInt(pos, pos);
}

bool ReTron::CollisionSystem::HasHitBox(entt::entity entity)
{
	return _registry.has<HitBoxSpecComponent>(entity);
}

void ReTron::CollisionSystem::ResetHitBox(entt::entity entity)
{
	_registry.remove_if_exists<HitBoxComponent>(entity);
	_registry.emplace_or_replace<DirtyComponent>(entity);
}

bool ReTron::CollisionSystem::UpdateHitBox(entt::entity entity)
{
	bool hasHitBox = HasHitBox(entity);
	if (hasHitBox)
	{
		HitBoxComponent& hb = _registry.get_or_emplace<HitBoxComponent>(entity, HitBoxComponent{});
		if (!hb._body)
		{
			const HitBoxSpecComponent& spec = _registry.get<HitBoxSpecComponent>(entity);
			ff::PointFixedInt pos = _positionSystem.GetPosition(entity) * ::PIXEL_TO_WORLD_SCALE;

			b2BodyDef bodyDef{};
			bodyDef.userData = ::UserDataFromEntity(entity);
			bodyDef.position.Set(pos.x, pos.y);
			bodyDef.angle = (float)_positionSystem.GetRotation(entity) * ff::DEG_TO_RAD_F;
			bodyDef.allowSleep = false;
			bodyDef.fixedRotation = true;
			bodyDef.type = b2_dynamicBody;

			hb._body = _world.CreateBody(&bodyDef);
		}
		else if (_registry.has<DirtyComponent>(entity))
		{
			ff::PointFixedInt pos = _positionSystem.GetPosition(entity) * ::PIXEL_TO_WORLD_SCALE;
			float angle = (float)_positionSystem.GetRotation(entity) * ff::DEG_TO_RAD_F;
			hb._body->SetTransform(b2Vec2(pos.x, pos.y), angle);
		}

		if (!hb._fixture)
		{
			const HitBoxSpecComponent& spec = _registry.get<HitBoxSpecComponent>(entity);
			ff::PointFixedInt scale = _positionSystem.GetScale(entity);
			ff::PointFixedInt tl = spec._topLeft * scale;
			ff::PointFixedInt br = tl + spec._size * scale;

			std::array<b2Vec2, 4> points =
			{
				b2Vec2{ tl.x, tl.y },
				b2Vec2{ br.x, tl.y },
				b2Vec2{ br.x, br.y },
				b2Vec2{ tl.x, br.y },
			};

			b2PolygonShape shape;
			shape.Set(points.data(), 4);

			b2FixtureDef fixtureDef;
			fixtureDef.userData = UserDataFromEntity(entity);
			fixtureDef.shape = &shape;
			fixtureDef.isSensor = true;
			hb._fixture = hb._body->CreateFixture(&fixtureDef);
		}
	}

	_registry.remove_if_exists<DirtyComponent>(entity);

	return hasHitBox;
}

void ReTron::CollisionSystem::UpdateDirtyHitBoxes()
{
	auto dirtyView = _registry.view<DirtyComponent>();
	for (entt::entity entity : dirtyView)
	{
		UpdateHitBox(entity);
	}
}

void ReTron::CollisionSystem::OnHitBoxRemoved(entt::registry& registry, entt::entity entity)
{
	HitBoxComponent& hb = _registry.get<HitBoxComponent>(entity);
	b2Body* body = hb._body;

	if (body)
	{
		hb._body = nullptr;
		hb._fixture = nullptr;
		body->GetWorld()->DestroyBody(body);
	}
}

void ReTron::CollisionSystem::OnHitBoxSpecChanged(entt::registry& registry, entt::entity entity)
{
	ResetHitBox(entity);
}

void ReTron::CollisionSystem::OnPositionChanged(entt::entity entity)
{
	if (HasHitBox(entity))
	{
		_registry.emplace_or_replace<DirtyComponent>(entity);
	}
}

void ReTron::CollisionSystem::OnScaleChanged(entt::entity entity)
{
	ResetHitBox(entity);
}

bool ReTron::CollisionSystem::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	entt::entity entityA = ::EntityFromUserData(fixtureA->GetUserData());
	entt::entity entityB = ::EntityFromUserData(fixtureB->GetUserData());

	const HitBoxSpecComponent& specA = _registry.get<HitBoxSpecComponent>(entityA);
	const HitBoxSpecComponent& specB = _registry.get<HitBoxSpecComponent>(entityB);

	HitBoxType typeA = std::min(specA._type, specB._type);
	HitBoxType typeB = std::max(specA._type, specB._type);

	if (typeA != typeB && typeA != HitBoxType::None && typeB != HitBoxType::None)
	{
		switch (typeA)
		{
		case HitBoxType::Player:
			return typeB == HitBoxType::Enemy ||
				typeB == HitBoxType::Electrode ||
				typeB == HitBoxType::EnemyBullet;

		case HitBoxType::Enemy:
			return typeB == HitBoxType::Electrode ||
				typeB == HitBoxType::PlayerBullet;

		case HitBoxType::Electrode:
			return typeB == HitBoxType::PlayerBullet ||
				typeB == HitBoxType::EnemyBullet;

		case HitBoxType::PlayerBullet:
			return typeB == HitBoxType::EnemyBullet;
		}
	}

	return false;
}
