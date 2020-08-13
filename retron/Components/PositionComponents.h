#pragma once

namespace ReTron
{
	enum class HitBoxType
	{
		None,
		Player,
		Enemy,
		Electrode,
		PlayerBullet,
		EnemyBullet,
	};

	class PositionComponents
	{
	public:
		PositionComponents(entt::registry& registry);

		void Remove(entt::entity entity);

		void SetPosition(entt::entity entity, const ff::PointFixedInt& value);
		const ff::PointFixedInt& GetPosition(entt::entity entity);

		void SetHitBox(entt::entity entity, const ff::PointFixedInt& topLeft, const ff::PointFixedInt& size, HitBoxType type);
		const ff::RectFixedInt& GetHitBox(entt::entity entity);
		const ff::RectFixedInt* GetOldHitBox(entt::entity entity);

		void SetBoundingBox(entt::entity entity, const ff::PointFixedInt& topLeft, const ff::PointFixedInt& size);
		const ff::RectFixedInt& GetBoundingBox(entt::entity entity);
		const ff::RectFixedInt* GetOldBoundingBox(entt::entity entity);

		entt::sink<void(PositionComponents&, entt::entity)> PositionChangedSink();
		entt::sink<void(PositionComponents&, entt::entity)> HitBoxChangedSink();
		entt::sink<void(PositionComponents&, entt::entity)> BoundingBoxChangedSink();

	private:
		void OnPositionChanged(entt::registry& registry, entt::entity entity);
		void OnHitBoxChanged(entt::registry& registry, entt::entity entity);
		void OnBoundingBoxChanged(entt::registry& registry, entt::entity entity);

		entt::registry& _registry;
		std::forward_list<entt::scoped_connection> _connections;

		entt::sigh<void(PositionComponents&, entt::entity)> _positionChangedSignal;
		entt::sigh<void(PositionComponents&, entt::entity)> _hitBoxChangedSignal;
		entt::sigh<void(PositionComponents&, entt::entity)> _boundingBoxChangedSignal;
	};
}
