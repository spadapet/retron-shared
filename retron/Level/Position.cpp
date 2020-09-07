#include "pch.h"
#include "Graph/Render/PixelRenderer.h"
#include "Level/Position.h"

struct PositionComponent
{
	ff::PointFixedInt _position;
};

struct VelocityComponent
{
	ff::PointFixedInt _velocity;
};

struct DirectionComponent
{
	ff::PointFixedInt _direction;
};

struct ScaleComponent
{
	ff::PointFixedInt _scale;
};

struct RotationComponent
{
	ff::FixedInt _rotation;
};

ReTron::Position::Position(entt::registry& registry)
	: _registry(registry)
{
	_connections.emplace_front(_registry.on_construct<PositionComponent>().connect<&Position::OnPositionChanged>(this));
	_connections.emplace_front(_registry.on_update<PositionComponent>().connect<&Position::OnPositionChanged>(this));
}

void ReTron::Position::SetPosition(entt::entity entity, const ff::PointFixedInt& value)
{
	if (value != GetPosition(entity))
	{
		_registry.emplace_or_replace<PositionComponent>(entity, value);
	}
}

ff::PointFixedInt ReTron::Position::GetPosition(entt::entity entity)
{
	PositionComponent* c = _registry.try_get<PositionComponent>(entity);
	return c ? c->_position : ff::PointFixedInt::Zeros();
}

void ReTron::Position::SetVelocity(entt::entity entity, const ff::PointFixedInt& value)
{
	if (value != GetVelocity(entity))
	{
		_registry.emplace_or_replace<VelocityComponent>(entity, value);
	}
}

ff::PointFixedInt ReTron::Position::GetVelocity(entt::entity entity)
{
	VelocityComponent* c = _registry.try_get<VelocityComponent>(entity);
	return c ? c->_velocity : ff::PointFixedInt::Zeros();
}

void ReTron::Position::SetDirection(entt::entity entity, const ff::PointFixedInt& value)
{
	if (value != GetDirection(entity))
	{
		_registry.emplace_or_replace<DirectionComponent>(entity, value);
	}
}

const ff::PointFixedInt ReTron::Position::GetDirection(entt::entity entity)
{
	DirectionComponent* c = _registry.try_get<DirectionComponent>(entity);
	return c ? c->_direction : ff::PointFixedInt::Zeros();
}

void ReTron::Position::SetScale(entt::entity entity, const ff::PointFixedInt& value)
{
	if (value != GetScale(entity))
	{
		_registry.emplace_or_replace<ScaleComponent>(entity, value);
	}
}

ff::PointFixedInt ReTron::Position::GetScale(entt::entity entity)
{
	ScaleComponent* c = _registry.try_get<ScaleComponent>(entity);
	return c ? c->_scale : ff::PointFixedInt::Ones();
}

void ReTron::Position::SetRotation(entt::entity entity, ff::FixedInt value)
{
	if (value != GetRotation(entity))
	{
		_registry.emplace_or_replace<RotationComponent>(entity, value);
	}
}

ff::FixedInt ReTron::Position::GetRotation(entt::entity entity)
{
	RotationComponent* c = _registry.try_get<RotationComponent>(entity);
	return c ? c->_rotation : 0_f;
}

entt::sink<void(entt::entity)> ReTron::Position::PositionChanged()
{
	return _positionChanged;
}

entt::sink<void(entt::entity)> ReTron::Position::DirectionChanged()
{
	return _directionChanged;
}

entt::sink<void(entt::entity)> ReTron::Position::ScaleChanged()
{
	return _scaleChanged;
}

entt::sink<void(entt::entity)> ReTron::Position::RotationChanged()
{
	return _rotationChanged;
}

void ReTron::Position::RenderDebug(ff::PixelRendererActive& render)
{
	for (auto [entity, pc] : _registry.view<PositionComponent>().proxy())
	{
		render.DrawPaletteFilledRectangle(ff::RectFixedInt(pc._position.Offset(-1, -1), pc._position.Offset(1, 1)), 230);
	}
}

void ReTron::Position::OnPositionChanged(entt::registry& registry, entt::entity entity)
{
	_positionChanged.publish(entity);
}

void ReTron::Position::OnDirectionChanged(entt::registry& registry, entt::entity entity)
{
	_directionChanged.publish(entity);
}

void ReTron::Position::OnScaleChanged(entt::registry& registry, entt::entity entity)
{
	_scaleChanged.publish(entity);
}

void ReTron::Position::OnRotationChanged(entt::registry& registry, entt::entity entity)
{
	_rotationChanged.publish(entity);
}
