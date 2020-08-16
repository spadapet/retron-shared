#include "pch.h"
#include "Services/AppService.h"
#include "Services/GameService.h"
#include "States/LevelState.h"

ReTron::LevelState::LevelState(IGameService* gameService, const LevelSpec& levelSpec)
	: _gameService(gameService)
	, _levelSpec(levelSpec)
	, _targets(gameService->GetAppService(), RenderTargetTypes::Palette1)
	, _level(this)
{
}

ReTron::LevelState::~LevelState()
{
}

std::shared_ptr<ff::State> ReTron::LevelState::Advance(ff::AppGlobals* globals)
{
	_level.Advance(globals, GetCamera());

	return nullptr;
}

void ReTron::LevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_targets.Clear();

	_level.Render(globals,
		_gameService->GetAppService()->GetRenderer(),
		_targets.GetTarget(RenderTargetTypes::Palette1),
		_targets.GetDepth(RenderTargetTypes::Palette1),
		Constants::RENDER_LEVEL_RECT,
		GetCamera());

	_targets.Render(target);
}

ReTron::IGameService* ReTron::LevelState::GetGameService() const
{
	return _gameService;
}

const ReTron::LevelSpec& ReTron::LevelState::GetLevelSpec() const
{
	return _levelSpec;
}

ff::RectFixedInt ReTron::LevelState::GetCamera()
{
	return ff::RectFixedInt(Constants::RENDER_LEVEL_SIZE);
}
