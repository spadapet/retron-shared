#include "pch.h"
#include "Core/Player.h"
#include "Services/GameService.h"
#include "States/LevelState.h"

ReTron::LevelState::LevelState(IGameService* gameService, const LevelSpec& levelSpec, std::vector<Player*>&& players)
	: _gameService(gameService)
	, _levelSpec(levelSpec)
	, _players(std::move(players))
	, _targets(gameService->GetAppService(), RenderTargetTypes::Palette1)
	, _level(this)
{
}

ReTron::LevelState::~LevelState()
{
}

std::shared_ptr<ff::State> ReTron::LevelState::Advance(ff::AppGlobals* globals)
{
	_level.Advance(GetCamera());

	return nullptr;
}

void ReTron::LevelState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_targets.Clear();

	_level.Render(
		_targets.GetTarget(RenderTargetTypes::Palette1),
		_targets.GetDepth(RenderTargetTypes::Palette1),
		Constants::RENDER_RECT,
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

size_t ReTron::LevelState::GetPlayerCount() const
{
	return _players.size();
}

ReTron::Player& ReTron::LevelState::GetPlayer(size_t index) const
{
	return *_players[index];
}

ReTron::Player& ReTron::LevelState::GetPlayerOrCoop(size_t index) const
{
	Player& player = GetPlayer(index);
	return player._coop ? *player._coop : player;
}

ff::RectFixedInt ReTron::LevelState::GetCamera()
{
	return ff::RectFixedInt(Constants::RENDER_RECT);
}
