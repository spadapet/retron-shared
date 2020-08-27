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
	ff::RectFixedInt rect = ff::RectFixedInt::Zeros();

	b2Shape* shape = body->GetFixtureList()->GetShape();
	for (int i = 0; i < shape->GetChildCount(); i++)
	{
		b2AABB aabb;
		shape->ComputeAABB(&aabb, body->GetTransform(), i);
		ff::RectFixedInt childRect = ff::RectFixedInt(aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y) * ::WORLD_TO_PIXEL_SCALE;
		rect = !i ? childRect : rect = rect.Bound(childRect);
	}

	return rect;
}

ReTron::CollisionSystem::CollisionSystem(entt::registry& registry, PositionSystem& positionSystem, EntitySystem& entitySystem)
	: _positionSystem(positionSystem)
	, _entitySystem(entitySystem)
	, _registry(registry)
	, _world(b2Vec2(0, 0))
{
	_world.SetAllowSleeping(false);
	_world.SetContactFilter(this);

	_connections.emplace_front(_registry.on_destroy<HitBoxComponent>().connect<&CollisionSystem::OnHitBoxRemoved>(this));
	_connections.emplace_front(_registry.on_construct<HitBoxSpecComponent>().connect<&CollisionSystem::OnHitBoxSpecChanged>(this));
	_connections.emplace_front(_registry.on_update<HitBoxSpecComponent>().connect<&CollisionSystem::OnHitBoxSpecChanged>(this));
	_connections.emplace_front(entitySystem.EntityCreated().connect<&CollisionSystem::OnEntityCreated>(this));
	_connections.emplace_front(positionSystem.PositionChanged().connect<&CollisionSystem::OnPositionChanged>(this));
	_connections.emplace_front(positionSystem.RotationChanged().connect<&CollisionSystem::OnPositionChanged>(this));
	_connections.emplace_front(positionSystem.DirectionChanged().connect<&CollisionSystem::OnScaleChanged>(this));
	_connections.emplace_front(positionSystem.ScaleChanged().connect<&CollisionSystem::OnScaleChanged>(this));
}

size_t ReTron::CollisionSystem::DetectCollisions()
{
	UpdateDirtyHitBoxes();

	_world.Step(ff::SECONDS_PER_ADVANCE_F, 1, 1);
	_collisions.clear();

	for (const b2Contact* i = _world.GetContactList(); i; i = i->GetNext())
	{
		if (i->IsEnabled() && i->IsTouching())
		{
			entt::entity entityA = ::EntityFromUserData(i->GetFixtureA()->GetUserData());
			entt::entity entityB = ::EntityFromUserData(i->GetFixtureB()->GetUserData());

			if (GetHitBoxType(entityA) <= GetHitBoxType(entityB))
			{
				_collisions.emplace_back(entityA, entityB);
			}
			else
			{
				_collisions.emplace_back(entityB, entityA);
			}
		}
	}

	return _collisions.size();
}

size_t ReTron::CollisionSystem::GetCollisionCount() const
{
	return _collisions.size();
}

std::pair<entt::entity, entt::entity> ReTron::CollisionSystem::GetCollision(size_t index) const
{
	return _collisions[index];
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

void ReTron::CollisionSystem::RenderDebug(ff::PixelRendererActive& render)
{
	for (auto [entity, hb] : _registry.view<HitBoxComponent>().proxy())
	{
		ff::RectFixedInt rect = GetHitBox(entity);
		int color = 252;
		for (const b2ContactEdge* i = hb._body->GetContactList(); i; i = i->next)
		{
			if (i->contact->IsEnabled() && i->contact->IsTouching())
			{
				color = 232;
				break;
			}
		}

		render.DrawPaletteOutlineRectangle(rect, color, 1_f);
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

static b2BodyType GetBodyType(ReTron::EntityHitBoxType type)
{
	switch (type)
	{
	case ReTron::EntityHitBoxType::Obstacle:
	case ReTron::EntityHitBoxType::Level:
		return b2_staticBody;

	default:
		return b2_dynamicBody;
	}
}

HitBoxComponent* ReTron::CollisionSystem::UpdateHitBox(entt::entity entity)
{
	EntityHitBoxType type = GetHitBoxType(entity);
	if (type != EntityHitBoxType::None)
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
			bodyDef.type = ::GetBodyType(type);

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
			ff::PointFixedInt dir = _positionSystem.GetDirection(entity);
			ff::PointFixedInt scale = _positionSystem.GetScale(entity) * ff::PointFixedInt(dir.x < 0_f ? -1 : 1, dir.y < 0_f ? -1 : 1) * ::PIXEL_TO_WORLD_SCALE;
			ff::PointFixedInt tl = spec.TopLeft() * scale;
			ff::PointFixedInt br = spec.BottomRight() * scale;

			b2PolygonShape polygonShape;
			b2ChainShape chainShape;
			b2FixtureDef fixtureDef;
			fixtureDef.userData = UserDataFromEntity(entity);
			fixtureDef.isSensor = true;

			if (type == EntityHitBoxType::Level && _entitySystem.GetType(entity) == EntityType::LevelBounds)
			{
				// Clockwise
				std::array<b2Vec2, 4> points =
				{
					b2Vec2{ tl.x, tl.y },
					b2Vec2{ br.x, tl.y },
					b2Vec2{ br.x, br.y },
					b2Vec2{ tl.x, br.y },
				};

				chainShape.CreateLoop(points.data(), 4);
				chainShape.m_radius = 0;
				fixtureDef.shape = &chainShape;
			}
			else
			{
				// Counter-clockwise
				std::array<b2Vec2, 4> points =
				{
					b2Vec2{ tl.x, tl.y },
					b2Vec2{ tl.x, br.y },
					b2Vec2{ br.x, br.y },
					b2Vec2{ br.x, tl.y },
				};

				polygonShape.Set(points.data(), 4);
				polygonShape.m_radius = 0;
				fixtureDef.shape = &polygonShape;
			}

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

void ReTron::CollisionSystem::OnHitBoxRemoved(entt::registry& registry, entt::entity entity)
{
	HitBoxComponent& hb = _registry.get<HitBoxComponent>(entity);
	hb._body->GetWorld()->DestroyBody(hb._body);
	hb._body = nullptr;
}

void ReTron::CollisionSystem::OnHitBoxSpecChanged(entt::registry& registry, entt::entity entity)
{
	ResetHitBox(entity);
}

void ReTron::CollisionSystem::OnEntityCreated(entt::entity entity)
{
	DirtyHitBox(entity);
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
