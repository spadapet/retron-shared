#include "pch.h"
#include "Graph/Render/PixelRenderer.h"
#include "Level/CollisionSystem.h"
#include "Level/EntitySystem.h"
#include "Level/PositionSystem.h"

static const ff::FixedInt PIXEL_TO_WORLD_SCALE = 0.0625_f;
static const ff::FixedInt WORLD_TO_PIXEL_SCALE = 16_f;

struct BoxSpecComponent
{
	ff::RectFixedInt _rect;
	ReTron::EntityBoxType _type;
};

struct HitBoxSpecComponent : public BoxSpecComponent
{
};

struct BoundsBoxSpecComponent : public BoxSpecComponent
{
};

struct BoxComponent
{
	b2Body* _body;
};

struct HitBoxComponent : public BoxComponent
{
};

struct BoundsBoxComponent : public BoxComponent
{
};

struct HitDirtyComponent
{
};

struct BoundsDirtyComponent
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

static b2BodyType GetBodyType(ReTron::EntityBoxType type)
{
	switch (type)
	{
	case ReTron::EntityBoxType::Obstacle:
	case ReTron::EntityBoxType::Level:
		return b2_staticBody;

	default:
		return b2_dynamicBody;
	}
}

static ff::RectFixedInt GetBox(const b2Body* body)
{
	ff::RectFixedInt rect = ff::RectFixedInt::Zeros();

	const b2Shape* shape = body->GetFixtureList()->GetShape();
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
	, _hitFilter(this)
	, _boundsFilter(this)
	, _worlds{ b2World(b2Vec2(0, 0)), b2World(b2Vec2(0, 0)) }
{
	_worlds[0].SetAllowSleeping(false);
	_worlds[0].SetContactFilter(&_hitFilter);

	_worlds[1].SetAllowSleeping(false);
	_worlds[1].SetContactFilter(&_boundsFilter);

	_connections.emplace_front(_registry.on_destroy<HitBoxComponent>().connect<&CollisionSystem::OnHitBoxRemoved>(this));
	_connections.emplace_front(_registry.on_construct<HitBoxSpecComponent>().connect<&CollisionSystem::OnHitBoxSpecChanged>(this));
	_connections.emplace_front(_registry.on_update<HitBoxSpecComponent>().connect<&CollisionSystem::OnHitBoxSpecChanged>(this));

	_connections.emplace_front(_registry.on_destroy<BoundsBoxComponent>().connect<&CollisionSystem::OnBoundsBoxRemoved>(this));
	_connections.emplace_front(_registry.on_construct<BoundsBoxSpecComponent>().connect<&CollisionSystem::OnBoundsBoxSpecChanged>(this));
	_connections.emplace_front(_registry.on_update<BoundsBoxSpecComponent>().connect<&CollisionSystem::OnBoundsBoxSpecChanged>(this));

	_connections.emplace_front(entitySystem.EntityCreated().connect<&CollisionSystem::OnEntityCreated>(this));
	_connections.emplace_front(positionSystem.PositionChanged().connect<&CollisionSystem::OnPositionChanged>(this));
	_connections.emplace_front(positionSystem.RotationChanged().connect<&CollisionSystem::OnPositionChanged>(this));
	_connections.emplace_front(positionSystem.DirectionChanged().connect<&CollisionSystem::OnScaleChanged>(this));
	_connections.emplace_front(positionSystem.ScaleChanged().connect<&CollisionSystem::OnScaleChanged>(this));
}

size_t ReTron::CollisionSystem::DetectCollisions(CollisionBoxType collisionType)
{
	UpdateDirtyBoxes(collisionType);

	b2World& world = _worlds[(size_t)collisionType];
	world.Step(ff::SECONDS_PER_ADVANCE_F, 1, 1);
	_collisions.clear();

	for (const b2Contact* i = world.GetContactList(); i; i = i->GetNext())
	{
		if (i->IsEnabled() && i->IsTouching())
		{
			entt::entity entityA = ::EntityFromUserData(i->GetFixtureA()->GetUserData());
			entt::entity entityB = ::EntityFromUserData(i->GetFixtureB()->GetUserData());

			if (!_entitySystem.IsDeleted(entityA) && !_entitySystem.IsDeleted(entityB))
			{
				ff::RectFixedInt boxA = ::GetBox(i->GetFixtureA()->GetBody());
				ff::RectFixedInt boxB = ::GetBox(i->GetFixtureB()->GetBody());

				if (boxA.DoesIntersect(boxB))
				{
					if (GetBoxType(entityA, collisionType) <= GetBoxType(entityB, collisionType))
					{
						_collisions.emplace_back(entityA, entityB);
					}
					else
					{
						_collisions.emplace_back(entityB, entityA);
					}
				}
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

void ReTron::CollisionSystem::HitTest(const ff::RectFixedInt& bounds, std::vector<entt::entity>& entities, CollisionBoxType collisionType)
{
	UpdateDirtyBoxes(collisionType);

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

	ff::RectFixedInt worldBounds = bounds * ::PIXEL_TO_WORLD_SCALE;

	b2AABB aabb;
	aabb.lowerBound.Set(worldBounds.left, worldBounds.top);
	aabb.upperBound.Set(worldBounds.right, worldBounds.bottom);

	b2World& world = _worlds[(size_t)collisionType];
	world.QueryAABB(&callback, aabb);
}

std::tuple<entt::entity, ff::PointFixedInt, ff::PointFixedInt> ReTron::CollisionSystem::RayTest(
	const ff::PointFixedInt& start,
	const ff::PointFixedInt& end,
	CollisionBoxType collisionType)
{
	UpdateDirtyBoxes(collisionType);

	ff::PointFixedInt worldStart = start * ::PIXEL_TO_WORLD_SCALE;
	ff::PointFixedInt worldEnd = end * ::PIXEL_TO_WORLD_SCALE;

	class Callback : public b2RayCastCallback
	{
	public:
		Callback(CollisionSystem& collisionSystem, CollisionBoxType collisionType)
			: _collisionSystem(collisionSystem)
			, _collisionType(collisionType)
			, _entity(entt::null)
			, _point(ff::PointFixedInt::Zeros())
			, _fraction(0)
		{
		}

		std::tuple<entt::entity, ff::PointFixedInt, ff::PointFixedInt> GetResult() const
		{
			return std::make_tuple(_entity, _point, _normal);
		}

	private:
		virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
		{
			// Has to be closer than an existing hit
			if (_entity != entt::null && fraction >= _fraction)
			{
				return -1;
			}

			// Has to be a level box
			entt::entity entity = ::EntityFromUserData(fixture->GetUserData());
			EntityBoxType type = _collisionSystem.GetBoxType(entity, _collisionType);
			if (type != EntityBoxType::Level)
			{
				return -1;
			}

			_entity = entity;
			_point = ff::PointFixedInt(point.x, point.y) * ::WORLD_TO_PIXEL_SCALE;
			_normal = ff::PointFixedInt(normal.x, normal.y);
			_fraction = fraction;

			return fraction;
		}

		CollisionSystem& _collisionSystem;
		CollisionBoxType _collisionType;
		entt::entity _entity;
		ff::PointFixedInt _point;
		ff::PointFixedInt _normal;
		float _fraction;
	} callback(*this, collisionType);

	b2World& world = _worlds[(size_t)collisionType];
	world.RayCast(&callback, b2Vec2{ worldStart.x, worldStart.y }, b2Vec2{ worldEnd.x, worldEnd.y });

	return callback.GetResult();
}

std::tuple<bool, ff::PointFixedInt, ff::PointFixedInt> ReTron::CollisionSystem::RayTest(
	entt::entity entity,
	const ff::PointFixedInt& start,
	const ff::PointFixedInt& end,
	CollisionBoxType collisionType)
{
	const b2Body* body = UpdateBox(entity, collisionType);
	if (body)
	{
		ff::PointFixedInt worldStart = start * ::PIXEL_TO_WORLD_SCALE;
		ff::PointFixedInt worldEnd = end * ::PIXEL_TO_WORLD_SCALE;

		b2RayCastInput rayData{ b2Vec2{ worldStart.x, worldStart.y }, b2Vec2{ worldEnd.x, worldEnd.y }, 1.0f };
		const b2Shape* shape = body->GetFixtureList()->GetShape();

		for (int i = 0; i < shape->GetChildCount(); i++)
		{
			b2RayCastOutput result{};
			if (shape->RayCast(&result, rayData, body->GetTransform(), 0))
			{
				ff::PointFixedInt hit(
					ff::FixedInt(rayData.p1.x + result.fraction * (rayData.p2.x - rayData.p1.x)) * ::WORLD_TO_PIXEL_SCALE,
					ff::FixedInt(rayData.p1.y + result.fraction * (rayData.p2.y - rayData.p1.y)) * ::WORLD_TO_PIXEL_SCALE);

				return std::make_tuple(true, hit, ff::PointFixedInt(result.normal.x, result.normal.y));
			}
		}
	}

	return std::make_tuple(false, ff::PointFixedInt::Zeros(), ff::PointFixedInt::Zeros());
}

void ReTron::CollisionSystem::SetBox(entt::entity entity, const ff::RectFixedInt& rect, EntityBoxType type, CollisionBoxType collisionType)
{
	switch (collisionType)
	{
	default:
	case CollisionBoxType::HitBox:
		_registry.emplace_or_replace<HitBoxSpecComponent>(entity, rect, type);
		break;

	case CollisionBoxType::BoundsBox:
		_registry.emplace_or_replace<BoundsBoxSpecComponent>(entity, rect, type);
		break;
	}
}

void ReTron::CollisionSystem::ResetBoxToDefault(entt::entity entity, CollisionBoxType collisionType)
{
	switch (collisionType)
	{
	default:
	case CollisionBoxType::HitBox:
		_registry.remove_if_exists<HitBoxSpecComponent>(entity);
		break;

	case CollisionBoxType::BoundsBox:
		_registry.remove_if_exists<BoundsBoxSpecComponent>(entity);
		break;
	}

	ResetBox(entity, collisionType);
}

ff::RectFixedInt ReTron::CollisionSystem::GetBoxSpec(entt::entity entity, CollisionBoxType collisionType)
{
	ff::RectFixedInt box;

	switch (collisionType)
	{
	default:
	case CollisionBoxType::HitBox:
	{
		BoxSpecComponent* spec = _registry.try_get<HitBoxSpecComponent>(entity);
		box = spec ? spec->_rect : ReTron::GetHitBoxSpec(_entitySystem.GetType(entity));
	}
	break;

	case CollisionBoxType::BoundsBox:
	{
		BoxSpecComponent* spec = _registry.try_get<BoundsBoxSpecComponent>(entity);
		box = spec ? spec->_rect : ReTron::GetBoundsBoxSpec(_entitySystem.GetType(entity));
	}
	break;
	}

	return box * _positionSystem.GetScale(entity);
}

ff::RectFixedInt ReTron::CollisionSystem::GetBox(entt::entity entity, CollisionBoxType collisionType)
{
	b2Body* body = UpdateBox(entity, collisionType);
	if (body)
	{
		return ::GetBox(body);
	}

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	return ff::RectFixedInt(pos, pos);
}

ReTron::EntityBoxType ReTron::CollisionSystem::GetBoxType(entt::entity entity, CollisionBoxType collisionType)
{
	BoxSpecComponent* spec;

	switch (collisionType)
	{
	default:
	case CollisionBoxType::HitBox:
		spec = _registry.try_get<HitBoxSpecComponent>(entity);
		break;

	case CollisionBoxType::BoundsBox:
		spec = _registry.try_get<BoundsBoxSpecComponent>(entity);
		break;
	}

	return spec ? spec->_type : ReTron::GetBoxType(_entitySystem.GetType(entity));
}

template<typename BoxType>
static void RenderDebug(entt::registry& registry, ff::PixelRendererActive& render, int thickness, int color, int colorHit)
{
	for (auto [entity, hb] : registry.view<BoxType>().proxy())
	{
		bool hit = false;
		for (const b2ContactEdge* i = hb._body->GetContactList(); !hit && i; i = i->next)
		{
			hit = i->contact->IsEnabled() && i->contact->IsTouching();
		}

		ff::RectFixedInt rect = ::GetBox(hb._body);
		render.DrawPaletteOutlineRectangle(rect.Inflate(thickness / 2, thickness / 2), hit ? colorHit : color, thickness);
	}
}

void ReTron::CollisionSystem::RenderDebug(ff::PixelRendererActive& render)
{
	::RenderDebug<BoundsBoxComponent>(_registry, render, 2, 245, 248);
	::RenderDebug<HitBoxComponent>(_registry, render, 1, 252, 232);
}

void ReTron::CollisionSystem::ResetBox(entt::entity entity, CollisionBoxType collisionType)
{
	switch (collisionType)
	{
	default:
	case CollisionBoxType::HitBox:
		_registry.remove_if_exists<HitBoxComponent>(entity);
		break;

	case CollisionBoxType::BoundsBox:
		_registry.remove_if_exists<BoundsBoxComponent>(entity);
		break;
	}

	DirtyBox(entity, collisionType);
}

void ReTron::CollisionSystem::DirtyBox(entt::entity entity, CollisionBoxType collisionType)
{
	if (GetBoxType(entity, collisionType) != EntityBoxType::None)
	{
		switch (collisionType)
		{
		default:
		case CollisionBoxType::HitBox:
			_registry.emplace_or_replace<HitDirtyComponent>(entity);
			break;

		case CollisionBoxType::BoundsBox:
			_registry.emplace_or_replace<BoundsDirtyComponent>(entity);
			break;
		}
	}
}

template<typename BoxType, typename DirtyType>
b2Body* ReTron::CollisionSystem::UpdateBox(entt::entity entity, ReTron::EntityBoxType type, ReTron::CollisionBoxType collisionType)
{
	BoxComponent& hb = _registry.get_or_emplace<BoxType>(entity, BoxType{});
	b2World& world = _worlds[(size_t)collisionType];

	if (!hb._body)
	{
		ff::PointFixedInt pos = _positionSystem.GetPosition(entity) * ::PIXEL_TO_WORLD_SCALE;

		b2BodyDef bodyDef{};
		bodyDef.userData = ::UserDataFromEntity(entity);
		bodyDef.position.Set(pos.x, pos.y);
		bodyDef.angle = (float)_positionSystem.GetRotation(entity) * -ff::DEG_TO_RAD_F;
		bodyDef.allowSleep = false;
		bodyDef.fixedRotation = true;
		bodyDef.type = ::GetBodyType(type);

		hb._body = world.CreateBody(&bodyDef);
	}
	else if (_registry.has<DirtyType>(entity))
	{
		ff::PointFixedInt pos = _positionSystem.GetPosition(entity) * ::PIXEL_TO_WORLD_SCALE;
		float angle = (float)_positionSystem.GetRotation(entity) * -ff::DEG_TO_RAD_F;
		hb._body->SetTransform(b2Vec2(pos.x, pos.y), angle);
	}

	if (!hb._body->GetFixtureList())
	{
		ff::RectFixedInt spec = GetBoxSpec(entity, collisionType) * ::PIXEL_TO_WORLD_SCALE;
		ff::PointFixedInt tl = spec.TopLeft();
		ff::PointFixedInt br = spec.BottomRight();

		b2PolygonShape polygonShape;
		b2ChainShape chainShape;
		b2FixtureDef fixtureDef;
		fixtureDef.userData = UserDataFromEntity(entity);
		fixtureDef.isSensor = true;

		if (type == EntityBoxType::Level && _entitySystem.GetType(entity) == EntityType::LevelBounds)
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

	_registry.remove_if_exists<DirtyType>(entity);
	return hb._body;
}

b2Body* ReTron::CollisionSystem::UpdateBox(entt::entity entity, CollisionBoxType collisionType)
{
	EntityBoxType type = GetBoxType(entity, collisionType);
	if (type != EntityBoxType::None)
	{
		switch (collisionType)
		{
		default:
		case CollisionBoxType::HitBox:
			return UpdateBox<HitBoxComponent, HitDirtyComponent>(entity, type, collisionType);

		case CollisionBoxType::BoundsBox:
			return UpdateBox<BoundsBoxComponent, BoundsDirtyComponent>(entity, type, collisionType);
		}
	}
	else
	{
		switch (collisionType)
		{
		default:
		case CollisionBoxType::HitBox:
			_registry.remove_if_exists<HitBoxComponent>(entity);
			_registry.remove_if_exists<HitDirtyComponent>(entity);
			break;

		case CollisionBoxType::BoundsBox:
			_registry.remove_if_exists<BoundsBoxComponent>(entity);
			_registry.remove_if_exists<BoundsDirtyComponent>(entity);
			break;
		}

		return nullptr;
	}
}

void ReTron::CollisionSystem::UpdateDirtyBoxes(CollisionBoxType collisionType)
{
	switch (collisionType)
	{
	default:
	case CollisionBoxType::HitBox:
		for (entt::entity entity : _registry.view<HitDirtyComponent>())
		{
			UpdateBox(entity, CollisionBoxType::HitBox);
		}
		break;

	case CollisionBoxType::BoundsBox:
		for (entt::entity entity : _registry.view<BoundsDirtyComponent>())
		{
			UpdateBox(entity, CollisionBoxType::BoundsBox);
		}
		break;
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
	ResetBox(entity, CollisionBoxType::HitBox);
}

void ReTron::CollisionSystem::OnBoundsBoxRemoved(entt::registry& registry, entt::entity entity)
{
	BoundsBoxComponent& bb = _registry.get<BoundsBoxComponent>(entity);
	bb._body->GetWorld()->DestroyBody(bb._body);
	bb._body = nullptr;
}

void ReTron::CollisionSystem::OnBoundsBoxSpecChanged(entt::registry& registry, entt::entity entity)
{
	ResetBox(entity, CollisionBoxType::BoundsBox);
}

void ReTron::CollisionSystem::OnEntityCreated(entt::entity entity)
{
	DirtyBox(entity, CollisionBoxType::HitBox);
	DirtyBox(entity, CollisionBoxType::BoundsBox);
}

void ReTron::CollisionSystem::OnPositionChanged(entt::entity entity)
{
	DirtyBox(entity, CollisionBoxType::HitBox);
	DirtyBox(entity, CollisionBoxType::BoundsBox);
}

void ReTron::CollisionSystem::OnScaleChanged(entt::entity entity)
{
	ResetBox(entity, CollisionBoxType::HitBox);
	ResetBox(entity, CollisionBoxType::BoundsBox);
}

ReTron::CollisionSystem::HitFilter::HitFilter(CollisionSystem* collisionSystem)
	: _collisionSystem(collisionSystem)
{
}

bool ReTron::CollisionSystem::HitFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	entt::entity entityA = ::EntityFromUserData(fixtureA->GetUserData());
	entt::entity entityB = ::EntityFromUserData(fixtureB->GetUserData());

	return ReTron::CanHitBoxCollide(
		_collisionSystem->GetBoxType(entityA, CollisionBoxType::HitBox),
		_collisionSystem->GetBoxType(entityB, CollisionBoxType::HitBox));
}

ReTron::CollisionSystem::BoundsFilter::BoundsFilter(CollisionSystem* collisionSystem)
	: _collisionSystem(collisionSystem)
{
}

bool ReTron::CollisionSystem::BoundsFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	entt::entity entityA = ::EntityFromUserData(fixtureA->GetUserData());
	entt::entity entityB = ::EntityFromUserData(fixtureB->GetUserData());

	return ReTron::CanBoundsBoxCollide(
		_collisionSystem->GetBoxType(entityA, CollisionBoxType::BoundsBox),
		_collisionSystem->GetBoxType(entityB, CollisionBoxType::BoundsBox));
}
