#include "pch.h"
#include "Level/Entity.h"

ReTron::EntityBoxType ReTron::GetBoxType(EntityType type)
{
	static EntityBoxType types[] =
	{
		EntityBoxType::None, // None
		EntityBoxType::Player, // Player
		EntityBoxType::PlayerBullet, // PlayerBullet
		EntityBoxType::Bonus, // BonusWoman
		EntityBoxType::Bonus, // BonusMan
		EntityBoxType::Bonus, // BonusChild
		EntityBoxType::Enemy, // Grunt
		EntityBoxType::Enemy, // Hulk
		EntityBoxType::Obstacle, // Electrode
		EntityBoxType::None, // LevelBorder
		EntityBoxType::None, // LevelBox
	};

	static_assert(_countof(types) == (size_t)EntityType::Count);
	assert((size_t)type < _countof(types));
	return types[(size_t)type];
}

const ff::RectFixedInt& ReTron::GetHitBoxSpec(EntityType type)
{
	static ff::RectFixedInt rects[] =
	{
		ff::RectFixedInt::Zeros(), // None
		ff::RectFixedInt(-5, -12, 5, 0), // Player
		ff::RectFixedInt(-10, 0, 0, 1), // PlayerBullet
		ff::RectFixedInt(-4, -8, 4, 0), // BonusWoman
		ff::RectFixedInt(-4, -8, 4, 0), // BonusMan
		ff::RectFixedInt(-4, -8, 4, 0), // BonusChild
		ff::RectFixedInt(-4, -10, 4, 0), // Grunt
		ff::RectFixedInt(-5, -8, 5, 0), // Hulk
		ff::RectFixedInt(-3, -3, 3, 3), // Electrode
		ff::RectFixedInt::Zeros(), // LevelBorder
		ff::RectFixedInt::Zeros(), // LevelBox
	};

	static_assert(_countof(rects) == (size_t)EntityType::Count);
	assert((size_t)type < _countof(rects));
	return rects[(size_t)type];
}

const ff::RectFixedInt& ReTron::GetBoundsBoxSpec(EntityType type)
{
	static ff::RectFixedInt rects[] =
	{
		ff::RectFixedInt::Zeros(), // None
		ff::RectFixedInt(-5, -12, 5, 0), // Player
		ff::RectFixedInt(-10, 0, 0, 1), // PlayerBullet
		ff::RectFixedInt(-5, -12, 5, 0), // BonusWoman
		ff::RectFixedInt(-5, -12, 5, 0), // BonusMan
		ff::RectFixedInt(-5, -12, 5, 0), // BonusChild
		ff::RectFixedInt(-5, -12, 5, 0), // Grunt
		ff::RectFixedInt(-5, -12, 5, 0), // Hulk
		ff::RectFixedInt(-4, -4, 4, 4), // Electrode
		ff::RectFixedInt::Zeros(), // LevelBorder
		ff::RectFixedInt::Zeros(), // LevelBox
	};

	static_assert(_countof(rects) == (size_t)EntityType::Count);
	assert((size_t)type < _countof(rects));
	return rects[(size_t)type];
}

bool ReTron::CanHitBoxCollide(EntityBoxType typeA, EntityBoxType typeB)
{
	if (typeA != typeB && typeA != EntityBoxType::None && typeB != EntityBoxType::None)
	{
		if (typeA > typeB)
		{
			std::swap(typeA, typeB);
		}

		switch (typeA)
		{
		case EntityBoxType::Player:
			return typeB == EntityBoxType::Bonus ||
				typeB == EntityBoxType::Enemy ||
				typeB == EntityBoxType::Obstacle ||
				typeB == EntityBoxType::EnemyBullet ||
				typeB == EntityBoxType::Level;

		case EntityBoxType::Bonus:
			return typeB == EntityBoxType::Enemy ||
				typeB == EntityBoxType::Obstacle ||
				typeB == EntityBoxType::EnemyBullet ||
				typeB == EntityBoxType::Level;

		case EntityBoxType::Enemy:
			return typeB == EntityBoxType::Obstacle ||
				typeB == EntityBoxType::PlayerBullet ||
				typeB == EntityBoxType::Level;

		case EntityBoxType::Obstacle:
			return typeB == EntityBoxType::PlayerBullet ||
				typeB == EntityBoxType::EnemyBullet;

		case EntityBoxType::PlayerBullet:
			return typeB == EntityBoxType::EnemyBullet ||
				typeB == EntityBoxType::Level;

		case EntityBoxType::EnemyBullet:
			return typeB == EntityBoxType::Level;
		}
	}

	return false;
}

bool ReTron::CanBoundsBoxCollide(EntityBoxType typeA, EntityBoxType typeB)
{
	return (typeA != typeB) && (typeA == EntityBoxType::Level || typeB == EntityBoxType::Level);
}
