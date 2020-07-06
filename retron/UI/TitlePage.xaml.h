#pragma once

#include "Core/Options.h"
#include "UI/NotifyPropertyChangedBase.h"

namespace ff
{
	class State;
}

namespace ReTron
{
	class IAppService;

	class TitlePageViewModel : public ff::NotifyPropertyChangedBase
	{
	public:
		TitlePageViewModel(const SystemOptions& systemOptions, const GameOptions& options);

		const char* GetPlayersText() const;
		void ChangePlayers(bool forward = true);

		const char* GetDifficultyText() const;
		void ChangeDifficulty(bool forward = true);

		const char* GetSoundText() const;
		void ChangeSound();

		const char* GetFullScreenText() const;
		void ChangeFullScreen();

	private:
		SystemOptions _systemOptions;
		GameOptions _gameOptions;
		Noesis::Ptr<Noesis::ICommand> _startGameCommand;
		Noesis::Ptr<Noesis::ICommand> _optionsCommand;
		Noesis::Ptr<Noesis::ICommand> _highScoresCommand;
		Noesis::Ptr<Noesis::ICommand> _aboutCommand;
		Noesis::Ptr<Noesis::ICommand> _playersCommand;
		Noesis::Ptr<Noesis::ICommand> _difficultyCommand;
		Noesis::Ptr<Noesis::ICommand> _soundCommand;
		Noesis::Ptr<Noesis::ICommand> _fullScreenCommand;

		NS_DECLARE_REFLECTION(TitlePageViewModel, ff::NotifyPropertyChangedBase);
	};

	class TitlePage : public Noesis::UserControl
	{
	public:
		TitlePage(IAppService* appService);
		virtual ~TitlePage() override;

		std::shared_ptr<ff::State> GetPendingState() const;
		TitlePageViewModel* GetViewModel() const;

		virtual bool ConnectEvent(BaseComponent* source, const char* event, const char* handler) override;

	private:
		IAppService* _appService;
		Noesis::Ptr<TitlePageViewModel> _viewModel;
		Noesis::Ptr<Noesis::ICommand> _focusLeftRightCommand;
		std::shared_ptr<ff::State> _pendingState;

		NS_DECLARE_REFLECTION(TitlePage, Noesis::UserControl);
	};
}
