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

		Count
	};

	enum class EntityHitBoxType
	{
		None,
		Player,
		Bonus,
		Enemy,
		Electrode,
		PlayerBullet,
		EnemyBullet,

		Count
	};

	EntityHitBoxType GetHitBoxType(EntityType type);
	const ff::RectFixedInt& GetHitBoxSpec(EntityType type);
	bool CanCollide(EntityHitBoxType typeA, EntityHitBoxType typeB);
}
