#pragma once

namespace ReTron
{
	// This is added to every entity as a component
	enum class EntityType
	{
		None,
		Player,
		PlayerBullet,
		BonusWoman,
		BonusMan,
		BonusChild,
		Grunt,
		Hulk,
		Electrode,
		LevelBounds,
		LevelBox,

		Count
	};

	enum class EntityBoxType
	{
		None,
		Player,
		Bonus,
		Enemy,
		Obstacle,
		PlayerBullet,
		EnemyBullet,
		Level,

		Count
	};

	EntityBoxType GetBoxType(EntityType type);

	const ff::RectFixedInt& GetHitBoxSpec(EntityType type);
	bool CanHitBoxCollide(EntityBoxType typeA, EntityBoxType typeB);

	const ff::RectFixedInt& GetBoundsBoxSpec(EntityType type);
	bool CanBoundsBoxCollide(EntityBoxType typeA, EntityBoxType typeB);

	class Entities
	{
	public:
		Entities(entt::registry& registry);

		entt::entity Create(EntityType type);
		bool DelayDelete(entt::entity entity);
		bool IsDeleted(entt::entity entity);
		void FlushDelete();

		size_t SortEntities();
		size_t GetEntityCount() const;
		entt::entity GetEntity(size_t index) const;
		EntityType GetType(entt::entity entity);

		entt::sink<void(entt::entity)> EntityCreated();
		entt::sink<void(entt::entity)> EntityDeleting();

	private:
		entt::registry& _registry;
		entt::sigh<void(entt::entity)> _entityCreated;
		entt::sigh<void(entt::entity)> _entityDeleting;
		bool _sortEntities;
	};
}
