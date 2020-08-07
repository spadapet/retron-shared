#include "pch.h"
#include "Services/LevelRenderService.h"

ReTron::LevelRenderService::LevelRenderService(ILevelService* levelService)
	: _levelService(levelService)
{
}
