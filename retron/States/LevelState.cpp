#include "pch.h"
#include "States/LevelState.h"

ReTron::LevelState::LevelState(IGameService* gameService, const DifficultySpec& diffSpec, const LevelSpec& levelSpec)
	: _gameService(gameService)
	, _level(diffSpec, levelSpec)
{
}

ReTron::LevelState::~LevelState()
{
}

std::shared_ptr<ff::State> ReTron::LevelState::Advance(ff::AppGlobals* globals)
{
	return ff::State::Advance(globals);
}

void ReTron::LevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
}

ReTron::IGameService* ReTron::LevelState::GetGameService() const
{
	return _gameService;
}

ReTron::Level& ReTron::LevelState::GetLevel()
{
	return _level;
}
