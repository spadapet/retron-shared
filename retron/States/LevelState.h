#pragma once

#include "Core/Level.h"
#include "Services/LevelService.h"
#include "State/State.h"

namespace ReTron
{
	struct DifficultySpec;
	struct LevelSpec;

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
		virtual Level& GetLevel() override;

	private:
		IGameService* _gameService;
		Level _level;
	};
}
