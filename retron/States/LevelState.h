#pragma once

#include "Core/Level.h"
#include "Core/RenderTargets.h"
#include "Services/LevelService.h"
#include "State/State.h"

namespace ReTron
{
	class IGameService;
	struct LevelSpec;

	class LevelState
		: public ff::State
		, public ILevelService
	{
	public:
		LevelState(IGameService* gameService, const LevelSpec& levelSpec);
		~LevelState();

		// State
		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;

		// ILevelService
		virtual IGameService* GetGameService() const override;
		virtual const LevelSpec& GetLevelSpec() const override;

	private:
		ff::RectFixedInt GetCamera();

		IGameService* _gameService;
		LevelSpec _levelSpec;
		RenderTargets _targets;
		Level _level;
	};
}
