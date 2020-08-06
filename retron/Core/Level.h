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

		uint32_t AllocEntity(Entity::Type type);
		void FreeEntity(uint32_t id);
		Entity& GetEntity(uint32_t id);
		const Entity& GetEntity(uint32_t id) const;
		uint32_t GetId(const Entity& entity) const;

		template<class T, class... Args> uint32_t AllocData(Args&&... args);
		template<class T> T& GetData(uint32_t index);
		template<class T> const T& GetData(uint32_t index) const;
		void FreeData(uint32_t index);

	private:
		uint32_t AllocDataBytes(size_t size);
		uint8_t* GetDataBytes(uint32_t index);
		const uint8_t* GetDataBytes(uint32_t index) const;

		ff::Vector<ReTron::Entity> _entities;
		ff::Vector<uint8_t> _data;
		ff::Vector<std::pair<uint32_t, uint32_t>> _dataMap; // [index] -> (offset in data, size)
		ff::Vector<uint32_t> _freeEntities;
		ff::Vector<uint32_t> _freeDataMap;
		size_t _freeDataSize;
	};
}

template<class T, class... Args>
uint32_t ReTron::Level::AllocData(Args&&... args)
{
	size_t size = ff::RoundUp(sizeof(T), std::max(alignof(T), 16));
	uint32_t index = AllocDataBytes(size);
	T* t = GetData<T>(index);
	::new(t) T(std::forward<Args>(args)...);
	return index;
}

template<class T>
T& ReTron::Level::GetData(uint32_t index)
{
	return *reinterpret_cast<T*>(GetDataBytes(index));
}

template<class T>
const T& ReTron::Level::GetData(uint32_t index) const
{
	return *reinterpret_cast<const T*>(GetDataBytes(index));
}
