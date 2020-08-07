#pragma once

namespace ReTron
{
	struct Entity
	{
		// 36 bytes per entity
		uint16_t _type;
		uint16_t _subType;
		uint32_t _state;
		uint32_t _levelData;
		uint32_t _logicCounter;
		uint32_t _renderCounter;
		ff::PointFixedInt _position;
		ff::PointFixedInt _velocity;

		typedef size_t ID;
		static const ID Null = ff::INVALID_SIZE;

		enum class Type : uint16_t
		{
			None,
			Player,
			Grunt,
		};

		enum class Type_Player : uint16_t
		{
			Player0,
			Player1,
		};

		enum class Type_Grunt : uint16_t
		{
			None,
		};

		enum class State_Player : uint32_t
		{
			None,
		};

		enum class State_Grunt : uint32_t
		{
			None,
		};

		void Init(Type type);
		Type GetType() const;

		template<class T> T& Cast();
		template<class T> const T& Cast() const;
		template<class T> T* SafeCast();
		template<class T> const T* SafeCast() const;
	};

	template<Entity::Type TypeValue, class SubType, class State>
	struct Entity_T : public Entity
	{
		static const Entity::Type Type = TypeValue;

		SubType GetSubType() const
		{
			assert(GetType() == Type);
			return static_cast<SubType>(_subType);
		}

		State GetState() const
		{
			assert(GetType() == Type);
			return static_cast<State>(_state);
		}
	};

	struct Entity_Player : public Entity_T<Entity::Type::Player, Entity::Type_Player, Entity::State_Player>
	{
	};

	struct Entity_Grunt : public Entity_T<Entity::Type::Grunt, Entity::Type_Grunt, Entity::State_Grunt>
	{
	};
}

struct EntityLevelData
{
	virtual ~EntityLevelData();
};

template<class T>
T& ReTron::Entity::Cast()
{
	assert(GetType() == T::Type);
	return *static_cast<T*>(this);
}

template<class T>
const T& ReTron::Entity::Cast() const
{
	assert(GetType() == T::Type);
	return *static_cast<const T*>(this);
}

template<class T>
T* ReTron::Entity::SafeCast()
{
	return (GetType() == T::Type) ? static_cast<T*>(this) : nullptr;
}

template<class T>
const T* ReTron::Entity::SafeCast() const
{
	return (GetType() == T::Type) ? static_cast<const T*>(this) : nullptr;
}
