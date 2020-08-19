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

	class PositionSystem;

	class CollisionSystem : public b2ContactFilter
	{
	public:
		CollisionSystem(entt::registry& registry, PositionSystem& positionSystem);

		void DetectCollisions(std::vector<std::pair<entt::entity, entt::entity>>& pairs);
		void HitTest(ff::RectFixedInt bounds, std::vector<entt::entity>& entities);

		void SetHitBox(entt::entity entity, const ff::PointFixedInt& topLeft, const ff::PointFixedInt& size, HitBoxType type);
		ff::RectFixedInt GetHitBox(entt::entity entity);

	private:
		// b2ContactFilter
		virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;

		bool HasHitBox(entt::entity entity);
		void ResetHitBox(entt::entity entity);
		bool UpdateHitBox(entt::entity entity);
		void UpdateDirtyHitBoxes();

		void OnHitBoxRemoved(entt::registry& registry, entt::entity entity);
		void OnHitBoxSpecChanged(entt::registry& registry, entt::entity entity);
		void OnPositionChanged(entt::entity entity);
		void OnScaleChanged(entt::entity entity);

		PositionSystem& _positionSystem;
		entt::registry& _registry;
		b2World _world;
		std::forward_list<entt::scoped_connection> _connections;
	};
}
