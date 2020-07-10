#include "pch.h"
#include "Services/AppService.h"
#include "States/GameState.h"
#include "UI/DelegateCommand.h"
#include "UI/TitlePage.xaml.h"

NS_IMPLEMENT_REFLECTION(ReTron::TitlePageViewModel, "ReTron.TitlePageViewModel")
{
	NsProp("PlayersText", &ReTron::TitlePageViewModel::GetPlayersText);
	NsProp("DifficultyText", &ReTron::TitlePageViewModel::GetDifficultyText);
	NsProp("SoundText", &ReTron::TitlePageViewModel::GetSoundText);
	NsProp("FullScreenText", &ReTron::TitlePageViewModel::GetFullScreenText);

	NsProp("StartGameCommand", &ReTron::TitlePageViewModel::_startGameCommand);
	NsProp("OptionsCommand", &ReTron::TitlePageViewModel::_optionsCommand);
	NsProp("HighScoresCommand", &ReTron::TitlePageViewModel::_highScoresCommand);
	NsProp("AboutCommand", &ReTron::TitlePageViewModel::_aboutCommand);
	NsProp("PlayersCommand", &ReTron::TitlePageViewModel::_playersCommand);
	NsProp("DifficultyCommand", &ReTron::TitlePageViewModel::_difficultyCommand);
	NsProp("SoundCommand", &ReTron::TitlePageViewModel::_soundCommand);
	NsProp("FullScreenCommand", &ReTron::TitlePageViewModel::_fullScreenCommand);
}

ReTron::TitlePageViewModel::TitlePageViewModel(const SystemOptions& systemOptions, const GameOptions& options)
	: _systemOptions(systemOptions)
	, _gameOptions(options)
	, _startGameCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::StartGameCommand)))
	, _optionsCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::OptionsCommand)))
	, _highScoresCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::HighScoresCommand)))
	, _aboutCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::AboutCommand)))
	, _playersCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::PlayersCommand)))
	, _difficultyCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::DifficultyCommand)))
	, _soundCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::SoundCommand)))
	, _fullScreenCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::FullScreenCommand)))
{
}

const char* ReTron::TitlePageViewModel::GetPlayersText() const
{
	switch (_gameOptions._players)
	{
	default: return "";
	case GamePlayers::One: return "One";
	case GamePlayers::TwoTakeTurns: return "Two (Swap)";
	case GamePlayers::TwoTogether: return "Two (Co-Op)";
	}
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

const char* ReTron::TitlePageViewModel::GetSoundText() const
{
	return _systemOptions._soundOn ? "On" : "Off";
}

const char* ReTron::TitlePageViewModel::GetFullScreenText() const
{
	return _systemOptions._fullScreen ? "On" : "Off";
}

void ReTron::TitlePageViewModel::StartGameCommand(Noesis::BaseComponent* param)
{
}

void ReTron::TitlePageViewModel::OptionsCommand(Noesis::BaseComponent* param)
{
}

void ReTron::TitlePageViewModel::HighScoresCommand(Noesis::BaseComponent* param)
{
}

void ReTron::TitlePageViewModel::AboutCommand(Noesis::BaseComponent* param)
{
}

void ReTron::TitlePageViewModel::PlayersCommand(Noesis::BaseComponent* param)
{
	bool forward = !Noesis::Boxing::CanUnbox<bool>(param) || Noesis::Boxing::Unbox<bool>(param);
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

void ReTron::TitlePageViewModel::DifficultyCommand(Noesis::BaseComponent* param)
{
	bool forward = !Noesis::Boxing::CanUnbox<bool>(param) || Noesis::Boxing::Unbox<bool>(param);
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

void ReTron::TitlePageViewModel::SoundCommand(Noesis::BaseComponent* param)
{
	_systemOptions._soundOn = !_systemOptions._soundOn;
	OnPropertyChanged("SoundText");
}

void ReTron::TitlePageViewModel::FullScreenCommand(Noesis::BaseComponent* param)
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
	return false;
}
