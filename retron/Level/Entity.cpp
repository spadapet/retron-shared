#include "pch.h"
#include "Level/Entity.h"

ReTron::EntityHitBoxType ReTron::GetHitBoxType(EntityType type)
{
	static EntityHitBoxType types[] =
	{
		EntityHitBoxType::None, // None
		EntityHitBoxType::Player, // Player
		EntityHitBoxType::Bonus, // BonusWoman
		EntityHitBoxType::Bonus, // BonusMan
		EntityHitBoxType::Bonus, // BonusChild
		EntityHitBoxType::Enemy, // Grunt
		EntityHitBoxType::Enemy, // Hulk
		EntityHitBoxType::Electrode, // Electrode
	};

	static_assert(_countof(types) == (size_t)EntityType::Count);
	assert((size_t)type < _countof(types));
	return types[(size_t)type];
}

const ff::RectFixedInt& ReTron::GetHitBoxSpec(EntityType type)
{
	static ff::RectFixedInt rects[] =
	{
		ff::RectFixedInt(0, 0, 0, 0), // None
		ff::RectFixedInt(-4, -8, 4, 0), // Player
		ff::RectFixedInt(-4, -8, 4, 0), // BonusWoman
		ff::RectFixedInt(-4, -8, 4, 0), // BonusMan
		ff::RectFixedInt(-4, -8, 4, 0), // BonusChild
		ff::RectFixedInt(-3, -6, 3, 0), // Grunt
		ff::RectFixedInt(-5, -8, 5, 0), // Hulk
		ff::RectFixedInt(-3, -3, 3, 3), // Electrode
	};

	static_assert(_countof(rects) == (size_t)EntityType::Count);
	assert((size_t)type < _countof(rects));
	return rects[(size_t)type];
}

bool ReTron::CanCollide(EntityHitBoxType typeA, EntityHitBoxType typeB)
{
	if (typeA != typeB && typeA != EntityHitBoxType::None && typeB != EntityHitBoxType::None)
	{
		if (typeA > typeB)
		{
			std::swap(typeA, typeB);
		}

		switch (typeA)
		{
		case EntityHitBoxType::Player:
			return typeB == EntityHitBoxType::Bonus || 
				typeB == EntityHitBoxType::Enemy ||
				typeB == EntityHitBoxType::Electrode ||
				typeB == EntityHitBoxType::EnemyBullet;

		case EntityHitBoxType::Bonus:
			return typeB == EntityHitBoxType::Enemy ||
				typeB == EntityHitBoxType::Electrode ||
				typeB == EntityHitBoxType::EnemyBullet;

		case EntityHitBoxType::Enemy:
			return typeB == EntityHitBoxType::Electrode ||
				typeB == EntityHitBoxType::PlayerBullet;

		case EntityHitBoxType::Electrode:
			return typeB == EntityHitBoxType::PlayerBullet ||
				typeB == EntityHitBoxType::EnemyBullet;

		case EntityHitBoxType::PlayerBullet:
			return typeB == EntityHitBoxType::EnemyBullet;
		}
	}

	return false;
}
