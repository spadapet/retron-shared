#pragma once

namespace ff
{
	class PixelRendererActive;
}

namespace ReTron
{
	class Position
	{
	public:
		Position(entt::registry& registry);

		void SetPosition(entt::entity entity, const ff::PointFixedInt& value);
		ff::PointFixedInt GetPosition(entt::entity entity);

		void SetVelocity(entt::entity entity, const ff::PointFixedInt& value);
		ff::PointFixedInt GetVelocity(entt::entity entity);
		ff::FixedInt GetVelocityAsAngle(entt::entity entity);
		ff::FixedInt GetReverseVelocityAsAngle(entt::entity entity);

		void SetDirection(entt::entity entity, const ff::PointFixedInt& value);
		const ff::PointFixedInt GetDirection(entt::entity entity);

		void SetScale(entt::entity entity, const ff::PointFixedInt& value);
		ff::PointFixedInt GetScale(entt::entity entity);

		void SetRotation(entt::entity entity, ff::FixedInt value);
		ff::FixedInt GetRotation(entt::entity entity);

		entt::sink<void(entt::entity)> PositionChanged();
		entt::sink<void(entt::entity)> DirectionChanged();
		entt::sink<void(entt::entity)> ScaleChanged();
		entt::sink<void(entt::entity)> RotationChanged();

		void RenderDebug(ff::PixelRendererActive& render);

	private:
		void OnPositionChanged(entt::registry& registry, entt::entity entity);
		void OnDirectionChanged(entt::registry& registry, entt::entity entity);
		void OnScaleChanged(entt::registry& registry, entt::entity entity);
		void OnRotationChanged(entt::registry& registry, entt::entity entity);

		entt::registry& _registry;
		std::forward_list<entt::scoped_connection> _connections;

		entt::sigh<void(entt::entity)> _positionChanged;
		entt::sigh<void(entt::entity)> _directionChanged;
		entt::sigh<void(entt::entity)> _scaleChanged;
		entt::sigh<void(entt::entity)> _rotationChanged;
	};
}
