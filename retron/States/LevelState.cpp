#include "pch.h"
#include "States/LevelState.h"

ReTron::LevelState::LevelState(IGameService* gameService)
	: _gameService(gameService)
{
}

ReTron::LevelState::~LevelState()
{
}

std::shared_ptr<ff::State> ReTron::LevelState::Advance(ff::AppGlobals* globals)
{
	return std::shared_ptr<ff::State>();
}

void ReTron::LevelState::OnFrameRendering(ff::AppGlobals* globals, ff::AdvanceType type)
{
}

void ReTron::LevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
}

ReTron::IGameService* ReTron::LevelState::GetGameService() const
{
	return _gameService;
}
