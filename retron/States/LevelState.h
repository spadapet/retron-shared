#pragma once

#include "Core/LevelService.h"
#include "Core/RenderTargets.h"
#include "Level/Level.h"
#include "State/State.h"

namespace ReTron
{
	class IGameService;
	struct LevelSpec;
	struct Player;

	class LevelState
		: public ff::State
		, public ILevelService
	{
	public:
		LevelState(IGameService* gameService, const LevelSpec& levelSpec, std::vector<Player*>&& players);
		~LevelState();

		// State
		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;

		// ILevelService
		virtual IGameService* GetGameService() const override;
		virtual const LevelSpec& GetLevelSpec() const override;
		virtual size_t GetPlayerCount() const override;
		virtual Player& GetPlayer(size_t index) const override;
		virtual Player& GetPlayerOrCoop(size_t index) const override;

	private:
		ff::RectFixedInt GetCamera();

		IGameService* _gameService;
		LevelSpec _levelSpec;
		std::vector<Player*> _players;
		RenderTargets _targets;
		Level _level;
	};
}
