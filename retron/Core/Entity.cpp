#include "pch.h"
#include "Core/Entity.h"

EntityLevelData::~EntityLevelData()
{
}

void ReTron::Entity::Init(Type type)
{
	_type = static_cast<uint16_t>(type);
	_subType = 0;
	_state = 0;
	_levelData = ff::INVALID_DWORD;
	_logicCounter = 0;
	_renderCounter = 0;
	_position.x = 0;
	_position.y = 0;
	_velocity.x = 0;
	_velocity.y = 0;
}

ReTron::Entity::Type ReTron::Entity::GetType() const
{
	return static_cast<Type>(_type);
}
