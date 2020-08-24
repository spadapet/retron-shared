#pragma once

namespace ReTron
{
	class EntitySystem;
	class PositionSystem;
	enum class EntityHitBoxType;

	class CollisionSystem : public b2ContactFilter
	{
	public:
		CollisionSystem(entt::registry& registry, PositionSystem& positionSystem, EntitySystem& entitySystem);

		void DetectCollisions(std::vector<std::pair<entt::entity, entt::entity>>& pairs);
		void HitTest(ff::RectFixedInt bounds, std::vector<entt::entity>& entities);

		void SetHitBox(entt::entity entity, const ff::RectFixedInt& rect, EntityHitBoxType type);
		void ResetHitBoxToDefault(entt::entity entity);
		ff::RectFixedInt GetHitBox(entt::entity entity);

	private:
		// b2ContactFilter
		virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;

		const ff::RectFixedInt &GetHitBoxSpec(entt::entity entity);
		EntityHitBoxType GetHitBoxType(entt::entity entity);
		void ResetHitBox(entt::entity entity);
		void DirtyHitBox(entt::entity entity);
		bool UpdateHitBox(entt::entity entity);
		void UpdateDirtyHitBoxes();

		void OnEntityCreated(entt::registry& registry, entt::entity entity);
		void OnHitBoxRemoved(entt::registry& registry, entt::entity entity);
		void OnHitBoxSpecChanged(entt::registry& registry, entt::entity entity);
		void OnPositionChanged(entt::entity entity);
		void OnScaleChanged(entt::entity entity);

		PositionSystem& _positionSystem;
		EntitySystem& _entitySystem;
		entt::registry& _registry;
		b2World _world;
		std::forward_list<entt::scoped_connection> _connections;
	};
}
