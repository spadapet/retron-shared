#pragma once

namespace ff
{
	class PixelRendererActive;
}

namespace ReTron
{
	class Entities;
	class Position;
	enum class EntityBoxType;

	enum class CollisionBoxType
	{
		HitBox,
		BoundsBox,
		GruntAvoidBox,
		Count
	};

	class Collision
	{
	public:
		Collision(entt::registry& registry, Position& position, Entities& entities);

		const std::vector<std::pair<entt::entity, entt::entity>>& DetectCollisions(std::vector<std::pair<entt::entity, entt::entity>>& collisions, CollisionBoxType collisionType);
		const std::vector<entt::entity>& HitTest(const ff::RectFixedInt& bounds, std::vector<entt::entity>& entities, EntityBoxType boxTypeFilter, CollisionBoxType collisionType);
		std::tuple<entt::entity, ff::PointFixedInt, ff::PointFixedInt> RayTest(const ff::PointFixedInt& start, const ff::PointFixedInt& end, EntityBoxType boxTypeFilter, CollisionBoxType collisionType);
		std::tuple<bool, ff::PointFixedInt, ff::PointFixedInt> RayTest(entt::entity entity, const ff::PointFixedInt& start, const ff::PointFixedInt& end, CollisionBoxType collisionType);

		void SetBox(entt::entity entity, const ff::RectFixedInt& rect, EntityBoxType type, CollisionBoxType collisionType);
		void ResetBoxToDefault(entt::entity entity, CollisionBoxType collisionType);
		ff::RectFixedInt GetBoxSpec(entt::entity entity, CollisionBoxType collisionType);
		ff::RectFixedInt GetBox(entt::entity entity, CollisionBoxType collisionType);
		EntityBoxType GetBoxType(entt::entity entity, CollisionBoxType collisionType);

		void RenderDebug(ff::PixelRendererActive& render);

	private:
		class HitFilter : public b2ContactFilter
		{
		public:
			HitFilter(Collision* collision);

		private:
			virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;

			Collision* _collision;
		};

		class BoundsFilter : public b2ContactFilter
		{
		public:
			BoundsFilter(Collision* collision);

		private:
			virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;

			Collision* _collision;
		};

		void ResetBox(entt::entity entity, CollisionBoxType collisionType);
		void DirtyBox(entt::entity entity, CollisionBoxType collisionType);
		b2Body* UpdateBox(entt::entity entity, CollisionBoxType collisionType);
		void UpdateDirtyBoxes(CollisionBoxType collisionType);
		bool NeedsLevelBoxAvoidSkin(entt::entity entity, CollisionBoxType collisionType);

		void OnBoundsBoxRemoved(entt::registry& registry, entt::entity entity);
		void OnEntityCreated(entt::entity entity);
		void OnPositionChanged(entt::entity entity);
		void OnScaleChanged(entt::entity entity);

		template<typename BoxType, typename DirtyType> b2Body* UpdateBox(entt::entity entity, ReTron::EntityBoxType type, ReTron::CollisionBoxType collisionType);
		template<typename BoxType> void RenderDebug(ff::PixelRendererActive& render, CollisionBoxType collisionType, int thickness, int color, int colorHit);
		template<typename T> void OnBoxRemoved(entt::registry& registry, entt::entity entity);
		template<CollisionBoxType T> void OnBoxSpecChanged(entt::registry& registry, entt::entity entity);

		// Entities
		Position& _position;
		Entities& _entities;
		entt::registry& _registry;
		std::forward_list<entt::scoped_connection> _connections;

		// Box2d
		HitFilter _hitFilter;
		BoundsFilter _boundsFilter;
		std::array<b2World, (size_t)CollisionBoxType::Count> _worlds;
	};
}
