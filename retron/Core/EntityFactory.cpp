#include "pch.h"
#include "Core/EntityFactory.h"
#include "Services/LevelService.h"

ReTron::EntityFactory::EntityFactory(ILevelService* levelService)
	: _levelService(levelService)
{
}

void ReTron::EntityFactory::InitLevel()
{
	const LevelSpec& levelSpec = _levelService->GetLevelSpec();
	Level& level = _levelService->GetLevel();
}
