#include "pch.h"
#include "Graph/Render/PixelRenderer.h"
#include "Level/Collision.h"
#include "Level/Entities.h"
#include "Level/Position.h"

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

static bool DoesOverlap(const b2Fixture* f1, const b2Fixture* f2)
{
	for (int i1 = 0; i1 < f1->GetShape()->GetChildCount(); i1++)
	{
		for (int i2 = 0; i2 < f2->GetShape()->GetChildCount(); i2++)
		{
			if (b2TestOverlap(f1->GetShape(), i1, f2->GetShape(), i2, f1->GetBody()->GetTransform(), f2->GetBody()->GetTransform()))
			{
				return true;
			}
		}
	}

	return false;
}

ReTron::Collision::Collision(entt::registry& registry, Position& position, Entities& entities)
	: _position(position)
	, _entities(entities)
	, _registry(registry)
	, _hitFilter(this)
	, _boundsFilter(this)
	, _worlds{ b2World(b2Vec2(0, 0)), b2World(b2Vec2(0, 0)) }
{
	_worlds[0].SetAllowSleeping(false);
	_worlds[0].SetContactFilter(&_hitFilter);

	_worlds[1].SetAllowSleeping(false);
	_worlds[1].SetContactFilter(&_boundsFilter);

	_connections.emplace_front(_registry.on_destroy<HitBoxComponent>().connect<&Collision::OnHitBoxRemoved>(this));
	_connections.emplace_front(_registry.on_construct<HitBoxSpecComponent>().connect<&Collision::OnHitBoxSpecChanged>(this));
	_connections.emplace_front(_registry.on_update<HitBoxSpecComponent>().connect<&Collision::OnHitBoxSpecChanged>(this));

	_connections.emplace_front(_registry.on_destroy<BoundsBoxComponent>().connect<&Collision::OnBoundsBoxRemoved>(this));
	_connections.emplace_front(_registry.on_construct<BoundsBoxSpecComponent>().connect<&Collision::OnBoundsBoxSpecChanged>(this));
	_connections.emplace_front(_registry.on_update<BoundsBoxSpecComponent>().connect<&Collision::OnBoundsBoxSpecChanged>(this));

	_connections.emplace_front(entities.EntityCreated().connect<&Collision::OnEntityCreated>(this));
	_connections.emplace_front(position.PositionChanged().connect<&Collision::OnPositionChanged>(this));
	_connections.emplace_front(position.RotationChanged().connect<&Collision::OnPositionChanged>(this));
	_connections.emplace_front(position.DirectionChanged().connect<&Collision::OnScaleChanged>(this));
	_connections.emplace_front(position.ScaleChanged().connect<&Collision::OnScaleChanged>(this));
}

size_t ReTron::Collision::DetectCollisions(CollisionBoxType collisionType)
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

			if (::DoesOverlap(i->GetFixtureA(), i->GetFixtureB()))
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

	return _collisions.size();
}

size_t ReTron::Collision::GetCollisionCount() const
{
	return _collisions.size();
}

std::pair<entt::entity, entt::entity> ReTron::Collision::GetCollision(size_t index) const
{
	return _collisions[index];
}

const std::vector<entt::entity>& ReTron::Collision::HitTest(
	const ff::RectFixedInt& bounds,
	std::vector<entt::entity>& entities,
	EntityBoxType boxTypeFilter,
	CollisionBoxType collisionType)
{
	entities.clear();
	UpdateDirtyBoxes(collisionType);

	class Callback : public b2QueryCallback
	{
	public:
		Callback(Collision* owner, std::vector<entt::entity>& entities, EntityBoxType boxTypeFilter, CollisionBoxType collisionType)
			: _owner(owner)
			, _entities(entities)
			, _boxTypeFilter(boxTypeFilter)
			, _collisionType(collisionType)
		{
		}

		virtual bool ReportFixture(b2Fixture* fixture) override
		{
			entt::entity entity = ::EntityFromUserData(fixture->GetUserData());
			if (_boxTypeFilter == EntityBoxType::None || _boxTypeFilter == _owner->GetBoxType(entity, _collisionType))
			{
				_entities.push_back(entity);
			}

			return true;
		}

	private:
		Collision* _owner;
		std::vector<entt::entity>& _entities;
		EntityBoxType _boxTypeFilter;
		CollisionBoxType _collisionType;
	} callback(this, entities, boxTypeFilter, collisionType);

	ff::RectFixedInt worldBounds = bounds * ::PIXEL_TO_WORLD_SCALE;

	b2AABB aabb;
	aabb.lowerBound.Set(worldBounds.left, worldBounds.top);
	aabb.upperBound.Set(worldBounds.right, worldBounds.bottom);

	b2World& world = _worlds[(size_t)collisionType];
	world.QueryAABB(&callback, aabb);

	return entities;
}

std::tuple<entt::entity, ff::PointFixedInt, ff::PointFixedInt> ReTron::Collision::RayTest(
	const ff::PointFixedInt& start,
	const ff::PointFixedInt& end,
	CollisionBoxType collisionType)
{
	if (start == end)
	{
		return std::make_tuple(entt::null, ff::PointFixedInt::Zeros(), ff::PointFixedInt::Zeros());
	}

	UpdateDirtyBoxes(collisionType);

	ff::PointFixedInt worldStart = start * ::PIXEL_TO_WORLD_SCALE;
	ff::PointFixedInt worldEnd = end * ::PIXEL_TO_WORLD_SCALE;

	class Callback : public b2RayCastCallback
	{
	public:
		Callback(Collision& collision, CollisionBoxType collisionType)
			: _collision(collision)
			, _collisionType(collisionType)
			, _entity(entt::null)
			, _point(ff::PointFixedInt::Zeros())
			, _normal(ff::PointFixedInt::Zeros())
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
			EntityBoxType type = _collision.GetBoxType(entity, _collisionType);
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

		Collision& _collision;
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

std::tuple<bool, ff::PointFixedInt, ff::PointFixedInt> ReTron::Collision::RayTest(
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

void ReTron::Collision::SetBox(entt::entity entity, const ff::RectFixedInt& rect, EntityBoxType type, CollisionBoxType collisionType)
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

void ReTron::Collision::ResetBoxToDefault(entt::entity entity, CollisionBoxType collisionType)
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

ff::RectFixedInt ReTron::Collision::GetBoxSpec(entt::entity entity, CollisionBoxType collisionType)
{
	ff::RectFixedInt box;

	switch (collisionType)
	{
	default:
	case CollisionBoxType::HitBox:
	{
		BoxSpecComponent* spec = _registry.try_get<HitBoxSpecComponent>(entity);
		box = spec ? spec->_rect : ReTron::GetHitBoxSpec(_entities.GetType(entity));
	}
	break;

	case CollisionBoxType::BoundsBox:
	{
		BoxSpecComponent* spec = _registry.try_get<BoundsBoxSpecComponent>(entity);
		box = spec ? spec->_rect : ReTron::GetBoundsBoxSpec(_entities.GetType(entity));
	}
	break;
	}

	return box * _position.GetScale(entity);
}

ff::RectFixedInt ReTron::Collision::GetBox(entt::entity entity, CollisionBoxType collisionType)
{
	b2Body* body = UpdateBox(entity, collisionType);
	if (body)
	{
		return ::GetBox(body);
	}

	ff::PointFixedInt pos = _position.GetPosition(entity);
	return ff::RectFixedInt(pos, pos);
}

ReTron::EntityBoxType ReTron::Collision::GetBoxType(entt::entity entity, CollisionBoxType collisionType)
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

	return spec ? spec->_type : ReTron::GetBoxType(_entities.GetType(entity));
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

void ReTron::Collision::RenderDebug(ff::PixelRendererActive& render)
{
	::RenderDebug<BoundsBoxComponent>(_registry, render, 2, 245, 248);
	::RenderDebug<HitBoxComponent>(_registry, render, 1, 252, 232);
}

void ReTron::Collision::ResetBox(entt::entity entity, CollisionBoxType collisionType)
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

void ReTron::Collision::DirtyBox(entt::entity entity, CollisionBoxType collisionType)
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
b2Body* ReTron::Collision::UpdateBox(entt::entity entity, ReTron::EntityBoxType type, ReTron::CollisionBoxType collisionType)
{
	BoxComponent& hb = _registry.get_or_emplace<BoxType>(entity, BoxType{});
	if (!hb._body)
	{
		ff::PointFixedInt pos = _position.GetPosition(entity) * ::PIXEL_TO_WORLD_SCALE;

		b2BodyDef bodyDef{};
		bodyDef.userData = ::UserDataFromEntity(entity);
		bodyDef.position.Set(pos.x, pos.y);
		bodyDef.angle = (float)_position.GetRotation(entity) * -ff::DEG_TO_RAD_F;
		bodyDef.allowSleep = false;
		bodyDef.fixedRotation = true;
		bodyDef.type = ::GetBodyType(type);

		b2World& world = _worlds[(size_t)collisionType];
		hb._body = world.CreateBody(&bodyDef);
	}
	else if (_registry.has<DirtyType>(entity))
	{
		ff::PointFixedInt pos = _position.GetPosition(entity) * ::PIXEL_TO_WORLD_SCALE;
		float angle = (float)_position.GetRotation(entity) * -ff::DEG_TO_RAD_F;
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

		if (type == EntityBoxType::Level && _entities.GetType(entity) == EntityType::LevelBounds)
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
			fixtureDef.shape = &polygonShape;
		}

		b2Fixture* fixture = hb._body->CreateFixture(&fixtureDef);
		fixture->GetShape()->m_radius = 0;
	}

	_registry.remove_if_exists<DirtyType>(entity);
	return hb._body;
}

b2Body* ReTron::Collision::UpdateBox(entt::entity entity, CollisionBoxType collisionType)
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

void ReTron::Collision::UpdateDirtyBoxes(CollisionBoxType collisionType)
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

void ReTron::Collision::OnHitBoxRemoved(entt::registry& registry, entt::entity entity)
{
	HitBoxComponent& hb = _registry.get<HitBoxComponent>(entity);
	hb._body->GetWorld()->DestroyBody(hb._body);
	hb._body = nullptr;
}

void ReTron::Collision::OnHitBoxSpecChanged(entt::registry& registry, entt::entity entity)
{
	ResetBox(entity, CollisionBoxType::HitBox);
}

void ReTron::Collision::OnBoundsBoxRemoved(entt::registry& registry, entt::entity entity)
{
	BoundsBoxComponent& bb = _registry.get<BoundsBoxComponent>(entity);
	bb._body->GetWorld()->DestroyBody(bb._body);
	bb._body = nullptr;
}

void ReTron::Collision::OnBoundsBoxSpecChanged(entt::registry& registry, entt::entity entity)
{
	ResetBox(entity, CollisionBoxType::BoundsBox);
}

void ReTron::Collision::OnEntityCreated(entt::entity entity)
{
	DirtyBox(entity, CollisionBoxType::HitBox);
	DirtyBox(entity, CollisionBoxType::BoundsBox);
}

void ReTron::Collision::OnPositionChanged(entt::entity entity)
{
	DirtyBox(entity, CollisionBoxType::HitBox);
	DirtyBox(entity, CollisionBoxType::BoundsBox);
}

void ReTron::Collision::OnScaleChanged(entt::entity entity)
{
	ResetBox(entity, CollisionBoxType::HitBox);
	ResetBox(entity, CollisionBoxType::BoundsBox);
}

ReTron::Collision::HitFilter::HitFilter(Collision* collision)
	: _collision(collision)
{
}

bool ReTron::Collision::HitFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	entt::entity entityA = ::EntityFromUserData(fixtureA->GetUserData());
	entt::entity entityB = ::EntityFromUserData(fixtureB->GetUserData());

	return
		!_collision->_entities.IsDeleted(entityA) &&
		!_collision->_entities.IsDeleted(entityB) &&
		ReTron::CanHitBoxCollide(
			_collision->GetBoxType(entityA, CollisionBoxType::HitBox),
			_collision->GetBoxType(entityB, CollisionBoxType::HitBox));
}

ReTron::Collision::BoundsFilter::BoundsFilter(Collision* collision)
	: _collision(collision)
{
}

bool ReTron::Collision::BoundsFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	entt::entity entityA = ::EntityFromUserData(fixtureA->GetUserData());
	entt::entity entityB = ::EntityFromUserData(fixtureB->GetUserData());

	return
		!_collision->_entities.IsDeleted(entityA) &&
		!_collision->_entities.IsDeleted(entityB) &&
		ReTron::CanBoundsBoxCollide(
			_collision->GetBoxType(entityA, CollisionBoxType::BoundsBox),
			_collision->GetBoxType(entityB, CollisionBoxType::BoundsBox));
}
