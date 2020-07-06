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
		TitlePageViewModel(const GameOptions& options);

		const char* GetPlayersText() const;
		void ChangePlayers(bool forward = true);

		const char* GetDifficultyText() const;
		void ChangeDifficulty(bool forward = true);

	private:
		GameOptions _options;
		Noesis::Ptr<Noesis::ICommand> _playersCommand;
		Noesis::Ptr<Noesis::ICommand> _difficultyCommand;

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
		void OnLoaded(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args);
		void OnPlayersKeyDown(Noesis::BaseComponent* sender, const Noesis::KeyEventArgs& args);
		void OnDifficultyKeyDown(Noesis::BaseComponent* sender, const Noesis::KeyEventArgs& args);
		void OnOptionMouseEnter(Noesis::BaseComponent* sender, const Noesis::MouseEventArgs& args);

		IAppService* _appService;
		Noesis::Ptr<TitlePageViewModel> _viewModel;
		std::shared_ptr<ff::State> _pendingState;

		NS_DECLARE_REFLECTION(TitlePage, Noesis::UserControl);
	};
}
