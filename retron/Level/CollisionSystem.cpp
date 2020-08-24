#include "pch.h"
#include "Graph/Render/PixelRenderer.h"
#include "Level/CollisionSystem.h"
#include "Level/EntitySystem.h"
#include "Level/PositionSystem.h"

static const ff::FixedInt PIXEL_TO_WORLD_SCALE = 0.0625_f;
static const ff::FixedInt WORLD_TO_PIXEL_SCALE = 16_f;

struct HitBoxSpecComponent
{
	ff::RectFixedInt _rect;
	ReTron::EntityHitBoxType _type;
};

struct HitBoxComponent
{
	b2Body* _body;
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

ff::RectFixedInt GetHitBox(b2Body* body)
{
	b2AABB aabb;
	body->GetFixtureList()->GetShape()->ComputeAABB(&aabb, body->GetTransform(), 0);
	return ff::RectFixedInt(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y) * ::WORLD_TO_PIXEL_SCALE;
}

ReTron::CollisionSystem::CollisionSystem(entt::registry& registry, PositionSystem& positionSystem, EntitySystem& entitySystem)
	: _positionSystem(positionSystem)
	, _entitySystem(entitySystem)
	, _registry(registry)
	, _world(b2Vec2(0, 0))
{
	_world.SetAllowSleeping(false);
	_world.SetContactFilter(this);

	_connections.emplace_front(_registry.on_construct<EntityType>().connect<&CollisionSystem::OnEntityCreated>(this));
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

		if (GetHitBoxType(entityA) <= GetHitBoxType(entityB))
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

void ReTron::CollisionSystem::SetHitBox(entt::entity entity, const ff::RectFixedInt& rect, EntityHitBoxType type)
{
	_registry.emplace_or_replace<HitBoxSpecComponent>(entity, rect, type);
}

void ReTron::CollisionSystem::ResetHitBoxToDefault(entt::entity entity)
{
	_registry.remove_if_exists<HitBoxSpecComponent>(entity);
	ResetHitBox(entity);
}

ff::RectFixedInt ReTron::CollisionSystem::GetHitBox(entt::entity entity)
{
	HitBoxComponent* hb = UpdateHitBox(entity);
	if (hb)
	{
		return ::GetHitBox(hb->_body);
	}

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	return ff::RectFixedInt(pos, pos);
}

void ReTron::CollisionSystem::RenderDebug(ff::PixelRendererActive& render, const std::vector<std::pair<entt::entity, entt::entity>>& collisions)
{
	for (auto [entity, hb] : _registry.view<HitBoxComponent>().proxy())
	{
		if (hb._body)
		{
			ff::RectFixedInt rect = GetHitBox(entity);
			int color = hb._body->GetContactList() ? 232 : 252;
			render.DrawPaletteOutlineRectangle(rect, color, 1_f);
		}
	}
}

const ff::RectFixedInt& ReTron::CollisionSystem::GetHitBoxSpec(entt::entity entity)
{
	const HitBoxSpecComponent* spec = _registry.try_get<HitBoxSpecComponent>(entity);
	return spec ? spec->_rect : ReTron::GetHitBoxSpec(_entitySystem.GetType(entity));
}

ReTron::EntityHitBoxType ReTron::CollisionSystem::GetHitBoxType(entt::entity entity)
{
	HitBoxSpecComponent* spec = _registry.try_get<HitBoxSpecComponent>(entity);
	return spec ? spec->_type : ReTron::GetHitBoxType(_entitySystem.GetType(entity));
}

void ReTron::CollisionSystem::ResetHitBox(entt::entity entity)
{
	_registry.remove_if_exists<HitBoxComponent>(entity);
	DirtyHitBox(entity);
}

void ReTron::CollisionSystem::DirtyHitBox(entt::entity entity)
{
	if (GetHitBoxType(entity) != EntityHitBoxType::None)
	{
		_registry.emplace_or_replace<DirtyComponent>(entity);
	}
}

HitBoxComponent* ReTron::CollisionSystem::UpdateHitBox(entt::entity entity)
{
	if (GetHitBoxType(entity) != EntityHitBoxType::None)
	{
		HitBoxComponent& hb = _registry.get_or_emplace<HitBoxComponent>(entity, HitBoxComponent{});
		if (!hb._body)
		{
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

		if (!hb._body->GetFixtureList())
		{
			const ff::RectFixedInt& spec = GetHitBoxSpec(entity);
			ff::PointFixedInt scale = _positionSystem.GetScale(entity) * ::PIXEL_TO_WORLD_SCALE;
			ff::PointFixedInt tl = spec.TopLeft() * scale;
			ff::PointFixedInt br = spec.BottomRight() * scale;

			std::array<b2Vec2, 4> points =
			{
				b2Vec2{ tl.x, tl.y },
				b2Vec2{ br.x, tl.y },
				b2Vec2{ br.x, br.y },
				b2Vec2{ tl.x, br.y },
			};

			b2PolygonShape shape;
			shape.Set(points.data(), 4);
			shape.m_radius = 0;

			b2FixtureDef fixtureDef;
			fixtureDef.userData = UserDataFromEntity(entity);
			fixtureDef.shape = &shape;
			fixtureDef.isSensor = true;
			hb._body->CreateFixture(&fixtureDef);
		}

		_registry.remove_if_exists<DirtyComponent>(entity);
		return &hb;
	}
	else
	{
		_registry.remove_if_exists<HitBoxComponent>(entity);
		_registry.remove_if_exists<DirtyComponent>(entity);
		return nullptr;
	}
}

void ReTron::CollisionSystem::UpdateDirtyHitBoxes()
{
	for (entt::entity entity : _registry.view<DirtyComponent>())
	{
		UpdateHitBox(entity);
	}
}

void ReTron::CollisionSystem::OnEntityCreated(entt::registry& registry, entt::entity entity)
{
	DirtyHitBox(entity);
}

void ReTron::CollisionSystem::OnHitBoxRemoved(entt::registry& registry, entt::entity entity)
{
	HitBoxComponent& hb = _registry.get<HitBoxComponent>(entity);
	if (hb._body)
	{
		hb._body->GetWorld()->DestroyBody(hb._body);
		hb._body = nullptr;
	}
}

void ReTron::CollisionSystem::OnHitBoxSpecChanged(entt::registry& registry, entt::entity entity)
{
	ResetHitBox(entity);
}

void ReTron::CollisionSystem::OnPositionChanged(entt::entity entity)
{
	DirtyHitBox(entity);
}

void ReTron::CollisionSystem::OnScaleChanged(entt::entity entity)
{
	ResetHitBox(entity);
}

bool ReTron::CollisionSystem::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	entt::entity entityA = ::EntityFromUserData(fixtureA->GetUserData());
	entt::entity entityB = ::EntityFromUserData(fixtureB->GetUserData());

	return ReTron::CanCollide(GetHitBoxType(entityA), GetHitBoxType(entityB));
}
