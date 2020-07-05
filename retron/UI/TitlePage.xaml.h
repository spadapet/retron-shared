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

		GameDifficulty GetDifficulty() const;
		void SetDifficulty(GameDifficulty value);
		const char* GetDifficultyText() const;
		Noesis::ICommand* GetDifficultyCommand() const;
		void ChangeDifficulty(bool forward = true);

	private:
		GameOptions _options;
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
		void OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args);
		void OnDifficultyKeyDown(Noesis::BaseComponent* sender, const Noesis::KeyEventArgs& args);
		void OnDifficultyMouseEnter(Noesis::BaseComponent* sender, const Noesis::MouseEventArgs& args);

		IAppService* _appService;
		Noesis::Ptr<TitlePageViewModel> _viewModel;
		std::shared_ptr<ff::State> _pendingState;

		NS_DECLARE_REFLECTION(TitlePage, Noesis::UserControl);
	};
}
