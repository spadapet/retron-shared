#include "pch.h"
#include "Core/Level.h"

ReTron::Level::Level()
	: _firstFreeEntity(Entity::Null)
	, _firstFreeDataMap(ff::INVALID_SIZE)
	, _freeDataSize(0)
{
	_entities.Reserve(256);
	_dataMap.Reserve(256);
	_data.Reserve(1024);
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
		_dataMap = rhs._dataMap;
		_data = rhs._data;
		_firstFreeEntity = rhs._firstFreeEntity;
		_firstFreeDataMap = rhs._firstFreeDataMap;
		_freeDataSize = rhs._freeDataSize;
	}

	return *this;
}

ReTron::Level& ReTron::Level::operator=(Level&& rhs)
{
	if (this != &rhs)
	{
		_entities = std::move(rhs._entities);
		_dataMap = std::move(rhs._dataMap);
		_data = std::move(rhs._data);
		_firstFreeEntity = rhs._firstFreeEntity;
		_firstFreeDataMap = rhs._firstFreeDataMap;
		_freeDataSize = rhs._freeDataSize;
	}

	return *this;
}

ReTron::Entity::ID ReTron::Level::AllocEntity(Entity::Type type)
{
	if (_firstFreeEntity != Entity::Null)
	{
		Entity::ID id = _firstFreeEntity;
		Entity& entity = GetEntity(_firstFreeEntity);
		_firstFreeEntity = static_cast<Entity::ID>(entity._levelData);
		entity.Init(type);
		return id;
	}
	else
	{
		Entity entity;
		entity.Init(type);
		_entities.PushEmplace(std::move(entity));
		return _entities.Size() - 1;
	}
}

void ReTron::Level::FreeEntity(Entity::ID id)
{
	Entity& entity = GetEntity(id);
	entity.Init(Entity::Type::None);
	entity._levelData = static_cast<uint32_t>(_firstFreeEntity);
	_firstFreeEntity = id;
}

ReTron::Entity& ReTron::Level::GetEntity(Entity::ID id)
{
	return _entities[id];
}

const ReTron::Entity& ReTron::Level::GetEntity(Entity::ID id) const
{
	return _entities[id];
}

ReTron::Entity::ID ReTron::Level::GetId(const Entity& entity) const
{
	return &entity - _entities.ConstData();
}

void ReTron::Level::FreeData(Entity& entity)
{
	if (entity._levelData != ff::INVALID_DWORD)
	{
		size_t index = static_cast<size_t>(entity._levelData);
		_freeDataSize += _dataMap[index].second;
		_dataMap[index] = SizePair{_firstFreeDataMap, 0};
		_firstFreeDataMap = index;

		if (_freeDataSize >= _data.Size() / 2)
		{
			CompressDataBytes();
		}
	}
}

size_t ReTron::Level::AllocDataBytes(size_t size)
{
	size_t index;
	if (_firstFreeDataMap != ff::INVALID_SIZE)
	{
		index = _firstFreeDataMap;
		_firstFreeDataMap = _dataMap[_firstFreeDataMap].first;
	}
	else
	{
		index = _dataMap.Size();
		_dataMap.PushEmplace(SizePair{});
	}

	_dataMap[index] = SizePair{_data.Size(), size};
	_data.InsertDefault(_data.Size(), size);

	return index;
}

uint8_t* ReTron::Level::GetDataBytes(size_t index)
{
	return &_data[_dataMap[index].first];
}

const uint8_t* ReTron::Level::GetDataBytes(size_t index) const
{
	return &_data[_dataMap[index].first];
}

void ReTron::Level::CompressDataBytes()
{
	ff::Vector<SizePair*> sortedDataMap;
	sortedDataMap.Reserve(_dataMap.Size());

	for (SizePair& i : _dataMap)
	{
		if (i.second)
		{
			sortedDataMap.Push(&i);
		}
	}

	std::sort(sortedDataMap.begin(), sortedDataMap.end(), [](SizePair* lhs, SizePair* rhs)
		{
			return lhs->first < rhs->first;
		});

	size_t pos = 0;
	for (SizePair* i : sortedDataMap)
	{
		std::memmove(_data.Data(pos), _data.Data(i->first), i->second);
		i->first = pos;
		pos += i->second;
	}

	_data.Delete(pos, _data.Size() - pos);
}
