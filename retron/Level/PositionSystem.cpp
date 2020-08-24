#include "pch.h"
#include "Graph/Render/PixelRenderer.h"
#include "Level/PositionSystem.h"

struct PositionComponent
{
	ff::PointFixedInt _pos;
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

ReTron::PositionSystem::PositionSystem(entt::registry& registry)
	: _registry(registry)
{
	_connections.emplace_front(_registry.on_construct<PositionComponent>().connect<&PositionSystem::OnPositionChanged>(this));
	_connections.emplace_front(_registry.on_update<PositionComponent>().connect<&PositionSystem::OnPositionChanged>(this));
}

void ReTron::PositionSystem::SetPosition(entt::entity entity, const ff::PointFixedInt& value)
{
	_registry.emplace_or_replace<PositionComponent>(entity, value);
}

ff::PointFixedInt ReTron::PositionSystem::GetPosition(entt::entity entity)
{
	PositionComponent* c = _registry.try_get<PositionComponent>(entity);
	return c ? c->_pos : ff::PointFixedInt::Zeros();
}

void ReTron::PositionSystem::SetDirection(entt::entity entity, const ff::PointFixedInt& value)
{
	_registry.emplace_or_replace<DirectionComponent>(entity, value);
}

const ff::PointFixedInt ReTron::PositionSystem::GetDirection(entt::entity entity)
{
	DirectionComponent* c = _registry.try_get<DirectionComponent>(entity);
	return c ? c->_direction : ff::PointFixedInt::Zeros();
}

void ReTron::PositionSystem::SetScale(entt::entity entity, const ff::PointFixedInt& value)
{
	_registry.emplace_or_replace<ScaleComponent>(entity, value);
}

ff::PointFixedInt ReTron::PositionSystem::GetScale(entt::entity entity)
{
	ScaleComponent* c = _registry.try_get<ScaleComponent>(entity);
	return c ? c->_scale : ff::PointFixedInt::Ones();
}

void ReTron::PositionSystem::SetRotation(entt::entity entity, ff::FixedInt value)
{
	_registry.emplace_or_replace<RotationComponent>(entity, 0);
}

ff::FixedInt ReTron::PositionSystem::GetRotation(entt::entity entity)
{
	RotationComponent* c = _registry.try_get<RotationComponent>(entity);
	return c ? c->_rotation : 0_f;
}

entt::sink<void(entt::entity)> ReTron::PositionSystem::PositionChanged()
{
	return _positionChanged;
}

entt::sink<void(entt::entity)> ReTron::PositionSystem::DirectionChanged()
{
	return _directionChanged;
}

entt::sink<void(entt::entity)> ReTron::PositionSystem::ScaleChanged()
{
	return _scaleChanged;
}

entt::sink<void(entt::entity)> ReTron::PositionSystem::RotationChanged()
{
	return _rotationChanged;
}

void ReTron::PositionSystem::RenderDebug(ff::PixelRendererActive& render)
{
	for (auto [entity, pc] : _registry.view<PositionComponent>().proxy())
	{
		render.DrawPaletteFilledRectangle(ff::RectFixedInt(pc._pos.Offset(-1, -1), pc._pos.Offset(1, 1)), 230);
	}
}

void ReTron::PositionSystem::OnPositionChanged(entt::registry& registry, entt::entity entity)
{
	_positionChanged.publish(entity);
}

void ReTron::PositionSystem::OnDirectionChanged(entt::registry& registry, entt::entity entity)
{
	_directionChanged.publish(entity);
}

void ReTron::PositionSystem::OnScaleChanged(entt::registry& registry, entt::entity entity)
{
	_scaleChanged.publish(entity);
}

void ReTron::PositionSystem::OnRotationChanged(entt::registry& registry, entt::entity entity)
{
	_rotationChanged.publish(entity);
}
