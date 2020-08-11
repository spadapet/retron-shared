#pragma once

#include "Core/EntityFactory.h"
#include "Core/GameSpec.h"
#include "Core/LevelAdvance.h"
#include "Core/LevelRender.h"
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
		virtual EntityFactory& GetEntityFactory() override;
		virtual ff::EntityDomain& GetEntityDomain() override;

	private:
		void InitLevel();

		IGameService* _gameService;
		DifficultySpec _diffSpec;
		LevelSpec _levelSpec;
		EntityFactory _entityFactory;
		LevelAdvance _advance;
		LevelRender _render;
		ff::EntityDomain _domain;
	};
}
