#pragma once

namespace ReTron
{
	// This is added to every entity as a component
	enum class EntityType
	{
		None,
		Player,
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
}
