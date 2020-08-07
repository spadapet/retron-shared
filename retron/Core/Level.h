#pragma once

#include "Core/Entity.h"

namespace ReTron
{
	class Level
	{
	public:
		Level();
		Level(const Level& rhs);
		Level(Level&& rhs);
		~Level();

		Level& operator=(const Level& rhs);
		Level& operator=(Level&& rhs);

		Entity::ID AllocEntity(Entity::Type type);
		void FreeEntity(Entity::ID id);
		Entity& GetEntity(Entity::ID id);
		const Entity& GetEntity(Entity::ID id) const;
		Entity::ID GetId(const Entity& entity) const;

		template<class T, class... Args> void AllocData(Entity& entity, Args&&... args);
		template<class T> T& GetData(const Entity& entity);
		template<class T> const T& GetData(const Entity& entity) const;
		void FreeData(Entity& entity);

	private:
		size_t AllocDataBytes(size_t size);
		uint8_t* GetDataBytes(size_t index);
		const uint8_t* GetDataBytes(size_t index) const;
		void CompressDataBytes();

		typedef std::pair<size_t, size_t> SizePair;

		ff::Vector<ReTron::Entity> _entities;
		ff::Vector<SizePair> _dataMap; // [index] -> (offset in data, size)
		ff::Vector<uint8_t> _data;
		Entity::ID _firstFreeEntity;
		size_t _firstFreeDataMap;
		size_t _freeDataSize;
	};
}

template<class T, class... Args>
void ReTron::Level::AllocData(Entity& entity, Args&&... args)
{
	FreeData(entity);

	size_t size = ff::RoundUp(sizeof(T), std::max(alignof(T), 16));
	size_t index = AllocDataBytes(size);
	::new(GetDataBytes(index)) T(std::forward<Args>(args)...);
	entity._levelData = static_cast<uint32_t>(index);
}

template<class T>
T& ReTron::Level::GetData(const Entity& entity)
{
	return *reinterpret_cast<T*>(GetDataBytes(static_cast<size_t>(entity._levelData)));
}

template<class T>
const T& ReTron::Level::GetData(const Entity& entity) const
{
	return *reinterpret_cast<const T*>(GetDataBytes(static_cast<size_t>(entity._levelData)));
}
