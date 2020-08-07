#pragma once

#include "Core/GameSpec.h"
#include "Core/Level.h"
#include "Services/LevelAdvanceService.h"
#include "Services/LevelRenderService.h"
#include "Services/LevelService.h"
#include "State/State.h"

namespace ReTron
{
	class LevelState
		: public ff::State
		, public ReTron::ILevelService
	{
	public:
		LevelState(IGameService* gameService, const DifficultySpec& diffSpec, const LevelSpec& levelSpec);
		~LevelState();

		// State
		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;

		// ILevelService
		virtual IGameService* GetGameService() const override;
		virtual const DifficultySpec& GetDifficultySpec() const override;
		virtual const LevelSpec& GetLevelSpec() const override;
		virtual const Level& GetLevel() const override;
		virtual Level& GetLevel() override;

	private:
		void InitLevel();

		IGameService* _gameService;
		DifficultySpec _diffSpec;
		LevelSpec _levelSpec;
		Level _level;
		LevelAdvanceService _advance;
		LevelRenderService _render;
	};
}
