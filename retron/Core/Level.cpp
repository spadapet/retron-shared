#include "pch.h"
#include "Core/Level.h"

ReTron::Level::Level()
	: _freeDataSize(0)
{
}

ReTron::Level::Level(const Level& rhs)
	: Level()
{
	*this = rhs;
}

ReTron::Level::Level(Level&& rhs)
	: Level()
{
	*this = std::move(rhs);
}

ReTron::Level::~Level()
{
}

ReTron::Level& ReTron::Level::operator=(const Level& rhs)
{
	if (this != &rhs)
	{
		_entities = rhs._entities;
		_data = rhs._data;
		_dataMap = rhs._dataMap;
		_freeEntities = rhs._freeEntities;
		_freeDataMap = rhs._freeDataMap;
		_freeDataSize = rhs._freeDataSize;
	}

	return *this;
}

ReTron::Level& ReTron::Level::operator=(Level&& rhs)
{
	if (this != &rhs)
	{
		_entities = std::move(rhs._entities);
		_data = std::move(rhs._data);
		_dataMap = std::move(rhs._dataMap);
		_freeEntities = std::move(rhs._freeEntities);
		_freeDataMap = std::move(rhs._freeDataMap);
		_freeDataSize = rhs._freeDataSize;
	}

	return *this;
}

uint32_t ReTron::Level::AllocEntity(Entity::Type type)
{
	uint32_t id = _entities.Size();

	if (_freeEntities.Size())
	{
		id = _freeEntities.Pop();
	}
	else
	{
		_entities.PushEmplace();
	}

	Entity& entity = GetEntity(id);

	return 0;
}

void ReTron::Level::FreeEntity(uint32_t id)
{
}

ReTron::Entity& ReTron::Level::GetEntity(uint32_t id)
{
	static Entity e;
	return e;
}

const ReTron::Entity& ReTron::Level::GetEntity(uint32_t id) const
{
	static Entity e;
	return e;
}

uint32_t ReTron::Level::GetId(const Entity& entity) const
{
	size_t i = &entity - _entities.ConstData();
	assert(i < _entities.Size());
	return static_cast<uint32_t>(i);
}

void ReTron::Level::FreeData(uint32_t index)
{
}

uint32_t ReTron::Level::AllocDataBytes(size_t size)
{
	return 0;
}

uint8_t* ReTron::Level::GetDataBytes(uint32_t index)
{
	return &_data[_dataMap[index].first];
}

const uint8_t* ReTron::Level::GetDataBytes(uint32_t index) const
{
	return &_data[_dataMap[index].first];
}
