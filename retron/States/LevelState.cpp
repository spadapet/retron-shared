#include "pch.h"
#include "States/LevelState.h"

ReTron::LevelState::LevelState(IGameService* gameService, const DifficultySpec& diffSpec, const LevelSpec& levelSpec)
	: _gameService(gameService)
	, _diffSpec(diffSpec)
	, _levelSpec(levelSpec)
	, _entityFactory(this)
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
	_advance.Advance();

	return ff::State::Advance(globals);
}

void ReTron::LevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_render.Render(target, depth);
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
	_entityFactory.InitLevel();
}
