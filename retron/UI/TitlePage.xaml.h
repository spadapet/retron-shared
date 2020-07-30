#pragma once

#include "Core/Options.h"
#include "Types/Event.h"
#include "UI/Utility/NotifyPropertyChangedBase.h"

namespace ff
{
	class IRenderTargetWindow;
	class State;
}

namespace ReTron
{
	class IAppService;

	class TitlePageViewModel : public ff::NotifyPropertyChangedBase
	{
	public:
		TitlePageViewModel(IAppService* appService);
		~TitlePageViewModel() override;

		void SetVisualStateRoot(Noesis::FrameworkElement* value);
		std::shared_ptr<ff::State> GetPendingState() const;

		const char* GetPlayersText() const;
		const char* GetDifficultyText() const;
		const char* GetSoundText() const;
		const char* GetFullScreenText() const;

	private:
		void StartGameCommand(Noesis::BaseComponent* param);
		void PlayersCommand(Noesis::BaseComponent* param);
		void DifficultyCommand(Noesis::BaseComponent* param);
		void SoundCommand(Noesis::BaseComponent* param);
		void FullScreenCommand(Noesis::BaseComponent* param);
		void StateBackCommand(Noesis::BaseComponent* param);
		void OnTargetSizeChanged();

		IAppService* _appService;
		ff::ComPtr<ff::IRenderTargetWindow> _target;
		ff::EventCookie _targetSizeChangedCookie;
		std::shared_ptr<ff::State> _pendingState;
		Noesis::FrameworkElement* _visualStateRoot;
		Noesis::Ptr<Noesis::ICommand> _startGameCommand;
		Noesis::Ptr<Noesis::ICommand> _playersCommand;
		Noesis::Ptr<Noesis::ICommand> _difficultyCommand;
		Noesis::Ptr<Noesis::ICommand> _soundCommand;
		Noesis::Ptr<Noesis::ICommand> _fullScreenCommand;
		Noesis::Ptr<Noesis::ICommand> _stateBackCommand;

		NS_DECLARE_REFLECTION(TitlePageViewModel, ff::NotifyPropertyChangedBase);
	};

	class TitlePage : public Noesis::UserControl
	{
	public:
		TitlePage(IAppService* appService);
		virtual ~TitlePage() override;

		TitlePageViewModel* GetViewModel() const;

	private:
		IAppService* _appService;
		Noesis::Ptr<TitlePageViewModel> _viewModel;

		NS_DECLARE_REFLECTION(TitlePage, Noesis::UserControl);
	};
}
