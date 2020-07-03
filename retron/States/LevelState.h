#pragma once

#include "Services/LevelService.h"
#include "State/State.h"

namespace ReTron
{
	class LevelState
		: public ff::State
		, public ReTron::ILevelService
	{
	public:
		LevelState(IGameService* gameService);
		~LevelState();

		// State
		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void OnFrameRendering(ff::AppGlobals* globals, ff::AdvanceType type) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;

		// ILevelService
		virtual IGameService* GetGameService() const override;

	private:
		IGameService* _gameService;
	};
}
