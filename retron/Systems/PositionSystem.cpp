#include "pch.h"
#include "Systems/PositionSystem.h"

struct PositionComponent
{
	ff::PointFixedInt _pos;
};

struct PositionToHitBoxComponent
{
	ff::PointFixedInt _topLeft;
	ff::PointFixedInt _size;
};

struct PositionToBoundingBoxComponent
{
	ff::PointFixedInt _topLeft;
	ff::PointFixedInt _size;
};

struct HitBoxComponent
{
	ff::RectFixedInt _box;
};

struct OldHitBoxComponent
{
	ff::RectFixedInt _box;
};

struct HitBoxTypeComponent
{
	ReTron::HitBoxType _type;
};

struct BoundingBoxComponent
{
	ff::RectFixedInt _box;
};

struct OldBoundingBoxComponent
{
	ff::RectFixedInt _box;
};

ReTron::PositionComponents::PositionComponents(entt::registry& registry)
	: _registry(registry)
{
	_registry.prepare<PositionComponent>();
	_registry.prepare<PositionToHitBoxComponent>();
	_registry.prepare<PositionToBoundingBoxComponent>();
	_registry.prepare<HitBoxComponent>();
	_registry.prepare<OldHitBoxComponent>();
	_registry.prepare<HitBoxTypeComponent>();
	_registry.prepare<BoundingBoxComponent>();
	_registry.prepare<OldBoundingBoxComponent>();

	_connections.emplace_front(_registry.on_construct<PositionComponent>().connect<&PositionComponents::OnPositionChanged>(this));
	_connections.emplace_front(_registry.on_update<PositionComponent>().connect<&PositionComponents::OnPositionChanged>(this));
	_connections.emplace_front(_registry.on_construct<HitBoxComponent>().connect<&PositionComponents::OnHitBoxChanged>(this));
	_connections.emplace_front(_registry.on_update<HitBoxComponent>().connect<&PositionComponents::OnHitBoxChanged>(this));
	_connections.emplace_front(_registry.on_construct<BoundingBoxComponent>().connect<&PositionComponents::OnBoundingBoxChanged>(this));
	_connections.emplace_front(_registry.on_update<BoundingBoxComponent>().connect<&PositionComponents::OnBoundingBoxChanged>(this));
}

void ReTron::PositionComponents::Remove(entt::entity entity)
{
	_registry.remove_if_exists<
		PositionComponent,
		PositionToHitBoxComponent,
		PositionToBoundingBoxComponent,
		HitBoxComponent,
		OldHitBoxComponent,
		HitBoxTypeComponent,
		BoundingBoxComponent,
		OldBoundingBoxComponent>(entity);
}

void ReTron::PositionComponents::SetPosition(entt::entity entity, const ff::PointFixedInt& value)
{
	_registry.emplace_or_replace<PositionComponent>(entity, value);
}

const ff::PointFixedInt& ReTron::PositionComponents::GetPosition(entt::entity entity)
{
	return _registry.get<PositionComponent>(entity)._pos;
}

void ReTron::PositionComponents::SetHitBox(entt::entity entity, const ff::PointFixedInt& topLeft, const ff::PointFixedInt& size, HitBoxType type)
{
	_registry.emplace_or_replace<HitBoxTypeComponent>(entity, type);
	_registry.emplace_or_replace<PositionToHitBoxComponent>(entity, topLeft, size);
}

const ff::RectFixedInt& ReTron::PositionComponents::GetHitBox(entt::entity entity)
{
	return _registry.get<HitBoxComponent>(entity)._box;
}

const ff::RectFixedInt* ReTron::PositionComponents::GetOldHitBox(entt::entity entity)
{
	const OldHitBoxComponent* box = _registry.try_get<OldHitBoxComponent>(entity);
	return box ? &box->_box : nullptr;
}

void ReTron::PositionComponents::SetBoundingBox(entt::entity entity, const ff::PointFixedInt& topLeft, const ff::PointFixedInt& size)
{
	_registry.emplace_or_replace<PositionToBoundingBoxComponent>(entity, topLeft, size);
}

const ff::RectFixedInt& ReTron::PositionComponents::GetBoundingBox(entt::entity entity)
{
	return _registry.get<BoundingBoxComponent>(entity)._box;
}

const ff::RectFixedInt* ReTron::PositionComponents::GetOldBoundingBox(entt::entity entity)
{
	const OldBoundingBoxComponent* box = _registry.try_get<OldBoundingBoxComponent>(entity);
	return box ? &box->_box : nullptr;
}

entt::sink<void(ReTron::PositionComponents&, entt::entity)> ReTron::PositionComponents::PositionChangedSink()
{
	return _positionChangedSignal;
}

entt::sink<void(ReTron::PositionComponents&, entt::entity)> ReTron::PositionComponents::HitBoxChangedSink()
{
	return _hitBoxChangedSignal;
}

entt::sink<void(ReTron::PositionComponents&, entt::entity)> ReTron::PositionComponents::BoundingBoxChangedSink()
{
	return _boundingBoxChangedSignal;
}

void ReTron::PositionComponents::OnPositionChanged(entt::registry& registry, entt::entity entity)
{
	ff::PointFixedInt pos = GetPosition(entity);

	// Update hit box
	{
		const PositionToHitBoxComponent* toHit = _registry.try_get<PositionToHitBoxComponent>(entity);
		if (toHit)
		{
			_registry.emplace_or_replace<HitBoxComponent>(entity, ff::RectFixedInt(pos - toHit->_topLeft, pos - toHit->_topLeft + toHit->_size));
		}
	}

	// Update bounding box
	{
		const PositionToBoundingBoxComponent* toBounds = _registry.try_get<PositionToBoundingBoxComponent>(entity);
		if (toBounds)
		{
			_registry.emplace_or_replace<BoundingBoxComponent>(entity, ff::RectFixedInt(pos - toBounds->_topLeft, pos - toBounds->_topLeft + toBounds->_size));
		}
	}

	_positionChangedSignal.publish(*this, entity);
}

void ReTron::PositionComponents::OnHitBoxChanged(entt::registry& registry, entt::entity entity)
{
	_hitBoxChangedSignal.publish(*this, entity);
	_registry.emplace_or_replace<OldHitBoxComponent>(entity, GetHitBox(entity));
}

void ReTron::PositionComponents::OnBoundingBoxChanged(entt::registry& registry, entt::entity entity)
{
	_boundingBoxChangedSignal.publish(*this, entity);
	_registry.emplace_or_replace<OldHitBoxComponent>(entity, GetBoundingBox(entity));
}
