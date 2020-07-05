#include "pch.h"
#include "Services/AppService.h"
#include "States/GameState.h"
#include "UI/DelegateCommand.h"
#include "UI/TitlePage.xaml.h"

NS_IMPLEMENT_REFLECTION(ReTron::TitlePageViewModel, "ReTron.TitlePageViewModel")
{
	NsProp("Difficulty", &ReTron::TitlePageViewModel::GetDifficulty, &ReTron::TitlePageViewModel::SetDifficulty);
	NsProp("DifficultyText", &ReTron::TitlePageViewModel::GetDifficultyText);
	NsProp("DifficultyCommand", &ReTron::TitlePageViewModel::GetDifficultyCommand);
}

ReTron::TitlePageViewModel::TitlePageViewModel(const GameOptions& options)
	: _options(options)
{
	_difficultyCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
			ChangeDifficulty();
		});
}

ReTron::GameDifficulty ReTron::TitlePageViewModel::GetDifficulty() const
{
	return _options._difficulty;
}

void ReTron::TitlePageViewModel::SetDifficulty(GameDifficulty value)
{
	if (value != _options._difficulty)
	{
		_options._difficulty = value;
		OnPropertyChanged("Difficulty");
		OnPropertyChanged("DifficultyText");
	}
}

const char* ReTron::TitlePageViewModel::GetDifficultyText() const
{
	switch (_options._difficulty)
	{
	default: return "";
	case GameDifficulty::Baby: return "Baby";
	case GameDifficulty::Easy: return "Easy";
	case GameDifficulty::Normal: return "Normal";
	case GameDifficulty::Hard: return "Hard";
	}
}

Noesis::ICommand* ReTron::TitlePageViewModel::GetDifficultyCommand() const
{
	return _difficultyCommand;
}

void ReTron::TitlePageViewModel::ChangeDifficulty(bool forward)
{
	if (forward)
	{
		SetDifficulty((_options._difficulty == GameDifficulty::Hard)
			? GameDifficulty::Easy
			: (GameDifficulty)((int)_options._difficulty + 1));
	}
	else
	{
		SetDifficulty((_options._difficulty == GameDifficulty::Easy)
			? GameDifficulty::Hard
			: (GameDifficulty)((int)_options._difficulty - 1));
	}
}

NS_IMPLEMENT_REFLECTION(ReTron::TitlePage, "ReTron.TitlePage")
{
	NsProp("ViewModel", &ReTron::TitlePage::GetViewModel);
}

ReTron::TitlePage::TitlePage(IAppService* appService)
	: _appService(appService)
	, _viewModel(*new TitlePageViewModel(appService->GetDefaultGameOptions()))
{
	Noesis::GUI::LoadComponent(this, "TitlePage.xaml");
}

ReTron::TitlePage::~TitlePage()
{
}

std::shared_ptr<ff::State> ReTron::TitlePage::GetPendingState() const
{
	return _pendingState;
}

ReTron::TitlePageViewModel* ReTron::TitlePage::GetViewModel() const
{
	return _viewModel;
}

bool ReTron::TitlePage::ConnectEvent(BaseComponent* source, const char* event, const char* handler)
{
	NS_CONNECT_EVENT(Noesis::Button, Click, OnClickPlayGame);
	NS_CONNECT_EVENT(Noesis::Button, KeyDown, OnDifficultyKeyDown);
	NS_CONNECT_EVENT(Noesis::Button, MouseEnter, OnDifficultyMouseEnter);

	NS_CONNECT_EVENT(Noesis::UserControl, Loaded, OnLoaded);

	return false;
}

void ReTron::TitlePage::OnLoaded(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	FindName<Noesis::Button>("difficultyButton")->Focus();
}

void ReTron::TitlePage::OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	_pendingState = std::make_shared<ReTron::GameState>(_appService, _appService->GetDefaultGameOptions());
}

void ReTron::TitlePage::OnDifficultyKeyDown(Noesis::BaseComponent* sender, const Noesis::KeyEventArgs& args)
{
	bool left = args.key == Noesis::Key::Key_Left || args.key == Noesis::Key::Key_GamepadLeft;
	bool right = args.key == Noesis::Key::Key_Right || args.key == Noesis::Key::Key_GamepadRight;

	if (left || right)
	{
		_viewModel->ChangeDifficulty(right);
	}
}

void ReTron::TitlePage::OnDifficultyMouseEnter(Noesis::BaseComponent* sender, const Noesis::MouseEventArgs& args)
{
	((Noesis::Button*)sender)->Focus();
}
