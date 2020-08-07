#include "pch.h"
#include "Services/LevelAdvanceService.h"

ReTron::LevelAdvanceService::LevelAdvanceService(ILevelService* levelService)
	: _levelService(levelService)
{
}
