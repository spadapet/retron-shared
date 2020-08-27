#pragma once

#include "Core/GameService.h"
#include "Core/GameSpec.h"
#include "Core/Options.h"
#include "Core/Player.h"
#include "Input/InputMapping.h"
#include "Resource/ResourceValue.h"
#include "State/State.h"

namespace ff
{
	class StateWrapper;
}

namespace ReTron
{
	class IAppService;

	class GameState
		: public ff::State
		, public IGameService
	{
	public:
		GameState(IAppService* appService);
		~GameState();

		// State
		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
		virtual size_t GetChildStateCount() override;
		virtual State* GetChildState(size_t index) override;

		// IGameService
		virtual IAppService* GetAppService() const override;
		virtual const GameOptions& GetGameOptions() const override;
		virtual const DifficultySpec& GetDifficultySpec() const override;
		virtual const ff::IInputEvents* GetInputEvents(const Player& player) override;
		virtual const ff::InputDevices& GetInputDevices(const Player& player) override;

		// Debug
		void RestartLevel();

	private:
		void InitInput();
		void InitPlayers();
		void InitLevel();
		Player& GetCoopPlayer();
		const LevelSpec& GetLevelSpec(size_t index) const;

		IAppService* _appService;
		GameOptions _gameOptions;
		DifficultySpec _diffSpec;
		LevelSetSpec _levelSetSpec;

		// Input
		ff::TypedResource<ff::IInputMapping> _gameInput;
		ff::InputDevices _gameInputDevices;
		std::array<ff::TypedResource<ff::IInputMapping>, Constants::MAX_PLAYERS> _playerInput;
		std::array<ff::InputDevices, Constants::MAX_PLAYERS> _playerInputDevices;
		std::array<Player, Constants::MAX_PLAYERS + 1> _players;

		// Level
		std::vector<std::shared_ptr<ff::StateWrapper>> _levelStates;
		size_t _playingLevelState;
	};
}
