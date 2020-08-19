#pragma once

#include "Core/GameSpec.h"
#include "Core/Options.h"
#include "Core/Player.h"
#include "Input/InputMapping.h"
#include "Resource/ResourceValue.h"
#include "Services/GameService.h"
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
		virtual ff::IInputEvents* GetPlayerInputEvents(size_t index) override;

		// Debug
		void RestartLevel();

	private:
		void InitInput();
		void InitPlayers();
		void InitLevel();

		IAppService* _appService;
		GameOptions _gameOptions;
		DifficultySpec _diffSpec;

		// Input
		ff::TypedResource<ff::IInputMapping> _gameInput;
		ff::InputDevices _gameInputDevices;
		std::array<ff::TypedResource<ff::IInputMapping>, Constants::MAX_PLAYERS> _playerInput;
		std::array<ff::InputDevices, Constants::MAX_PLAYERS> _playerInputDevices;
		std::array<Player, Constants::MAX_PLAYERS + 1> _players;

		// Level
		std::shared_ptr<ff::StateWrapper> _levelState;
		size_t _level;
	};
}
