#include "pch.h"
#include "Services/AppService.h"
#include "States/GameState.h"
#include "UI/DelegateCommand.h"
#include "UI/TitlePage.xaml.h"

NS_IMPLEMENT_REFLECTION(ReTron::TitlePageViewModel, "ReTron.TitlePageViewModel")
{
	NsProp("StartGameCommand", &ReTron::TitlePageViewModel::_startGameCommand);
	NsProp("OptionsCommand", &ReTron::TitlePageViewModel::_optionsCommand);
	NsProp("HighScoresCommand", &ReTron::TitlePageViewModel::_highScoresCommand);
	NsProp("AboutCommand", &ReTron::TitlePageViewModel::_aboutCommand);

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
	_startGameCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
		});

	_optionsCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
		});

	_highScoresCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
		});

	_aboutCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent*)
		{
		});

	_playersCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent* param)
		{
			bool forward = Noesis::Boxing::CanUnbox<bool>(param) && Noesis::Boxing::Unbox<bool>(param);
			ChangePlayers(forward);
		});

	_difficultyCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent* param)
		{
			bool forward = Noesis::Boxing::CanUnbox<bool>(param) && Noesis::Boxing::Unbox<bool>(param);
			ChangeDifficulty(forward);
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
	NsProp("FocusLeftRightCommand", &ReTron::TitlePage::_focusLeftRightCommand);
}

ReTron::TitlePage::TitlePage(IAppService* appService)
	: _appService(appService)
	, _viewModel(*new TitlePageViewModel(appService->GetSystemOptions(), appService->GetDefaultGameOptions()))
{
	_focusLeftRightCommand = Noesis::MakePtr<ff::DelegateCommand>(
		[this](Noesis::BaseComponent* param)
		{
			Noesis::ICommandSource* source = Noesis::DynamicCast<Noesis::ICommandSource*>(GetKeyboard()->GetFocused());
			if (source && source->GetCommand() && source->GetCommand()->CanExecute(param))
			{
				source->GetCommand()->Execute(param);
			}
		});


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
