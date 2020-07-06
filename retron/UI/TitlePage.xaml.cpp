#include "pch.h"
#include "Services/AppService.h"
#include "States/GameState.h"
#include "UI/DelegateCommand.h"
#include "UI/TitlePage.xaml.h"

NS_IMPLEMENT_REFLECTION(ReTron::TitlePageViewModel, "ReTron.TitlePageViewModel")
{
	NsProp("PlayersText", &ReTron::TitlePageViewModel::GetPlayersText);
	NsProp("PlayersCommand", &ReTron::TitlePageViewModel::_playersCommand);
	NsProp("DifficultyText", &ReTron::TitlePageViewModel::GetDifficultyText);
	NsProp("DifficultyCommand", &ReTron::TitlePageViewModel::_difficultyCommand);
	NsProp("SoundText", &ReTron::TitlePageViewModel::GetSoundText);
	NsProp("SoundCommand", &ReTron::TitlePageViewModel::_soundCommand);
	NsProp("FullScreenText", &ReTron::TitlePageViewModel::GetFullScreenText);
	NsProp("FullScreenCommand", &ReTron::TitlePageViewModel::_fullScreenCommand);
}

ReTron::TitlePageViewModel::TitlePageViewModel(const SystemOptions& systemOptions, const GameOptions& options)
	: _systemOptions(systemOptions)
	, _gameOptions(options)
{
	_playersCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
			ChangePlayers();
		});

	_difficultyCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
			ChangeDifficulty();
		});

	_soundCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
			ChangeSound();
		});

	_fullScreenCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
			ChangeFullScreen();
		});
}

const char* ReTron::TitlePageViewModel::GetPlayersText() const
{
	switch (_gameOptions._players)
	{
	default: return "";
	case GamePlayers::One: return "One";
	case GamePlayers::TwoTakeTurns: return "Two, Take Turns";
	case GamePlayers::TwoTogether: return "Two, Co-Op";
	}
}

void ReTron::TitlePageViewModel::ChangePlayers(bool forward)
{
	if (forward)
	{
		_gameOptions._players = (_gameOptions._players == GamePlayers::TwoTogether)
			? GamePlayers::One
			: (GamePlayers)((int)_gameOptions._players + 1);
	}
	else
	{
		_gameOptions._players = (_gameOptions._players == GamePlayers::One)
			? GamePlayers::TwoTogether
			: (GamePlayers)((int)_gameOptions._players - 1);
	}

	OnPropertyChanged("PlayersText");
}

const char* ReTron::TitlePageViewModel::GetDifficultyText() const
{
	switch (_gameOptions._difficulty)
	{
	default: return "";
	case GameDifficulty::Baby: return "Baby";
	case GameDifficulty::Easy: return "Easy";
	case GameDifficulty::Normal: return "Normal";
	case GameDifficulty::Hard: return "Hard";
	}
}

void ReTron::TitlePageViewModel::ChangeDifficulty(bool forward)
{
	if (forward)
	{
		_gameOptions._difficulty = (_gameOptions._difficulty == GameDifficulty::Hard)
			? GameDifficulty::Easy
			: (GameDifficulty)((int)_gameOptions._difficulty + 1);
	}
	else
	{
		_gameOptions._difficulty = (_gameOptions._difficulty == GameDifficulty::Easy)
			? GameDifficulty::Hard
			: (GameDifficulty)((int)_gameOptions._difficulty - 1);
	}

	OnPropertyChanged("DifficultyText");
}

const char* ReTron::TitlePageViewModel::GetSoundText() const
{
	return _systemOptions._soundOn ? "On" : "Off";
}

void ReTron::TitlePageViewModel::ChangeSound()
{
	_systemOptions._soundOn = !_systemOptions._soundOn;
	OnPropertyChanged("SoundText");
}

const char* ReTron::TitlePageViewModel::GetFullScreenText() const
{
	return _systemOptions._fullScreen ? "On" : "Off";
}

void ReTron::TitlePageViewModel::ChangeFullScreen()
{
	_systemOptions._fullScreen = !_systemOptions._fullScreen;
	OnPropertyChanged("FullScreenText");
}

NS_IMPLEMENT_REFLECTION(ReTron::TitlePage, "ReTron.TitlePage")
{
	NsProp("ViewModel", &ReTron::TitlePage::GetViewModel);
}

ReTron::TitlePage::TitlePage(IAppService* appService)
	: _appService(appService)
	, _viewModel(*new TitlePageViewModel(appService->GetSystemOptions(), appService->GetDefaultGameOptions()))
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
	NS_CONNECT_EVENT(Noesis::Button, KeyDown, OnPlayersKeyDown);
	NS_CONNECT_EVENT(Noesis::Button, KeyDown, OnDifficultyKeyDown);
	NS_CONNECT_EVENT(Noesis::Button, KeyDown, OnSoundKeyDown);
	NS_CONNECT_EVENT(Noesis::Button, KeyDown, OnFullScreenKeyDown);
	NS_CONNECT_EVENT(Noesis::Button, MouseEnter, OnOptionMouseEnter);

	NS_CONNECT_EVENT(Noesis::UserControl, Loaded, OnLoaded);

	return false;
}

void ReTron::TitlePage::OnLoaded(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	FindName<Noesis::Button>("startGameButton")->Focus();
}

void ReTron::TitlePage::OnPlayersKeyDown(Noesis::BaseComponent* sender, const Noesis::KeyEventArgs& args)
{
	bool left = args.key == Noesis::Key::Key_Left || args.key == Noesis::Key::Key_GamepadLeft;
	bool right = args.key == Noesis::Key::Key_Right || args.key == Noesis::Key::Key_GamepadRight;

	if (left || right)
	{
		_viewModel->ChangePlayers(right);
	}
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

void ReTron::TitlePage::OnSoundKeyDown(Noesis::BaseComponent* sender, const Noesis::KeyEventArgs& args)
{
	bool left = args.key == Noesis::Key::Key_Left || args.key == Noesis::Key::Key_GamepadLeft;
	bool right = args.key == Noesis::Key::Key_Right || args.key == Noesis::Key::Key_GamepadRight;

	if (left || right)
	{
		_viewModel->ChangeSound();
	}
}

void ReTron::TitlePage::OnFullScreenKeyDown(Noesis::BaseComponent* sender, const Noesis::KeyEventArgs& args)
{
	bool left = args.key == Noesis::Key::Key_Left || args.key == Noesis::Key::Key_GamepadLeft;
	bool right = args.key == Noesis::Key::Key_Right || args.key == Noesis::Key::Key_GamepadRight;

	if (left || right)
	{
		_viewModel->ChangeFullScreen();
	}
}

void ReTron::TitlePage::OnOptionMouseEnter(Noesis::BaseComponent* sender, const Noesis::MouseEventArgs& args)
{
	((Noesis::FrameworkElement*)sender)->Focus();
}
