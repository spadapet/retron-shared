#include "pch.h"
#include "States/LevelState.h"

Game::LevelState::LevelState(IGameService* gameService)
	: _gameService(gameService)
{
}

Game::LevelState::~LevelState()
{
}

std::shared_ptr<ff::State> Game::LevelState::Advance(ff::AppGlobals* globals)
{
	return std::shared_ptr<ff::State>();
}

void Game::LevelState::OnFrameRendering(ff::AppGlobals* globals, ff::AdvanceType type)
{
}

void Game::LevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
}

Game::IGameService* Game::LevelState::GetGameService() const
{
	return _gameService;
}
