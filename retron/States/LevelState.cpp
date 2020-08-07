#include "pch.h"
#include "States/LevelState.h"

ReTron::LevelState::LevelState(IGameService* gameService, const DifficultySpec& diffSpec, const LevelSpec& levelSpec)
	: _gameService(gameService)
	, _diffSpec(diffSpec)
	, _levelSpec(levelSpec)
	, _advance(this)
	, _render(this)
{
	InitLevel();
}

ReTron::LevelState::~LevelState()
{
}

std::shared_ptr<ff::State> ReTron::LevelState::Advance(ff::AppGlobals* globals)
{
	return std::shared_ptr<ff::State>();
}

void ReTron::LevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
}

ReTron::IGameService* ReTron::LevelState::GetGameService() const
{
	return _gameService;
}

const ReTron::DifficultySpec& ReTron::LevelState::GetDifficultySpec() const
{
	return _diffSpec;
}

const ReTron::LevelSpec& ReTron::LevelState::GetLevelSpec() const
{
	return _levelSpec;
}

const ReTron::Level& ReTron::LevelState::GetLevel() const
{
	return _level;
}

ReTron::Level& ReTron::LevelState::GetLevel()
{
	return _level;
}

void ReTron::LevelState::InitLevel()
{
}
