#include "pch.h"
#include "Core/LevelRender.h"

ReTron::LevelRender::LevelRender(ILevelService* levelService)
	: _levelService(levelService)
{
}

void ReTron::LevelRender::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
}
