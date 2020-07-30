#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Graph/RenderTarget/RenderTargetWindow.h"
#include "Services/AppService.h"
#include "States/GameState.h"
#include "UI/TitlePage.xaml.h"
#include "UI/Utility/DelegateCommand.h"

NS_IMPLEMENT_REFLECTION(ReTron::TitlePageViewModel, "ReTron.TitlePageViewModel")
{
	NsProp("PlayersText", &ReTron::TitlePageViewModel::GetPlayersText);
	NsProp("DifficultyText", &ReTron::TitlePageViewModel::GetDifficultyText);
	NsProp("SoundText", &ReTron::TitlePageViewModel::GetSoundText);
	NsProp("FullScreenText", &ReTron::TitlePageViewModel::GetFullScreenText);

	NsProp("StartGameCommand", &ReTron::TitlePageViewModel::_startGameCommand);
	NsProp("PlayersCommand", &ReTron::TitlePageViewModel::_playersCommand);
	NsProp("DifficultyCommand", &ReTron::TitlePageViewModel::_difficultyCommand);
	NsProp("SoundCommand", &ReTron::TitlePageViewModel::_soundCommand);
	NsProp("FullScreenCommand", &ReTron::TitlePageViewModel::_fullScreenCommand);
	NsProp("StateBackCommand", &ReTron::TitlePageViewModel::_stateBackCommand);
}

ReTron::TitlePageViewModel::TitlePageViewModel(IAppService* appService)
	: _appService(appService)
	, _target(appService->GetAppGlobals().GetTarget())
	, _visualStateRoot(nullptr)
	, _startGameCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::StartGameCommand)))
	, _playersCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::PlayersCommand)))
	, _difficultyCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::DifficultyCommand)))
	, _soundCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::SoundCommand)))
	, _fullScreenCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::FullScreenCommand)))
	, _stateBackCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::TitlePageViewModel::StateBackCommand)))
{
	_targetSizeChangedCookie = _target->SizeChanged().Add([this](ff::PointInt, double, int)
		{
			OnTargetSizeChanged();
		});
}

ReTron::TitlePageViewModel::~TitlePageViewModel()
{
	_target->SizeChanged().Remove(_targetSizeChangedCookie);
}

void ReTron::TitlePageViewModel::SetVisualStateRoot(Noesis::FrameworkElement* value)
{
	_visualStateRoot = value;
}

std::shared_ptr<ff::State> ReTron::TitlePageViewModel::GetPendingState() const
{
	return _pendingState;
}

const char* ReTron::TitlePageViewModel::GetPlayersText() const
{
	switch (_appService->GetDefaultGameOptions()._players)
	{
	default: return "";
	case GamePlayers::One: return "One";
	case GamePlayers::TwoTakeTurns: return "Two (Swap)";
	case GamePlayers::TwoTogether: return "Two (Co-Op)";
	}
}

const char* ReTron::TitlePageViewModel::GetDifficultyText() const
{
	switch (_appService->GetDefaultGameOptions()._difficulty)
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
	return _appService->GetSystemOptions()._soundOn ? "On" : "Off";
}

const char* ReTron::TitlePageViewModel::GetFullScreenText() const
{
	return _target->IsFullScreen() ? "On" : "Off";
}

void ReTron::TitlePageViewModel::StartGameCommand(Noesis::BaseComponent* param)
{
	_pendingState = std::make_shared<GameState>(_appService, _appService->GetDefaultGameOptions());
}

void ReTron::TitlePageViewModel::PlayersCommand(Noesis::BaseComponent* param)
{
	bool forward = !Noesis::Boxing::CanUnbox<bool>(param) || Noesis::Boxing::Unbox<bool>(param);
	GameOptions options = _appService->GetDefaultGameOptions();

	if (forward)
	{
		options._players = (options._players == GamePlayers::TwoTogether)
			? GamePlayers::One
			: (GamePlayers)((int)options._players + 1);
	}
	else
	{
		options._players = (options._players == GamePlayers::One)
			? GamePlayers::TwoTogether
			: (GamePlayers)((int)options._players - 1);
	}

	_appService->SetDefaultGameOptions(options);
	OnPropertyChanged("PlayersText");
}

void ReTron::TitlePageViewModel::DifficultyCommand(Noesis::BaseComponent* param)
{
	bool forward = !Noesis::Boxing::CanUnbox<bool>(param) || Noesis::Boxing::Unbox<bool>(param);
	GameOptions options = _appService->GetDefaultGameOptions();

	if (forward)
	{
		options._difficulty = (options._difficulty == GameDifficulty::Hard)
			? GameDifficulty::Easy
			: (GameDifficulty)((int)options._difficulty + 1);
	}
	else
	{
		options._difficulty = (options._difficulty == GameDifficulty::Easy)
			? GameDifficulty::Hard
			: (GameDifficulty)((int)options._difficulty - 1);
	}

	_appService->SetDefaultGameOptions(options);
	OnPropertyChanged("DifficultyText");
}

void ReTron::TitlePageViewModel::SoundCommand(Noesis::BaseComponent* param)
{
	SystemOptions options = _appService->GetSystemOptions();
	options._soundOn = !options._soundOn;
	_appService->SetSystemOptions(options);
	OnPropertyChanged("SoundText");
}

void ReTron::TitlePageViewModel::FullScreenCommand(Noesis::BaseComponent* param)
{
	_target->SetFullScreen(!_target->IsFullScreen());
}

void ReTron::TitlePageViewModel::StateBackCommand(Noesis::BaseComponent* param)
{
	Noesis::VisualStateGroupCollection* groups = Noesis::VisualStateManager::GetVisualStateGroups(_visualStateRoot);
	assertRet(groups);

	for (unsigned int i = 0; i < (unsigned int)groups->Count(); i++)
	{
		Noesis::VisualStateGroup* group = groups->Get(i);
		if (group->GetName() && !std::strcmp(group->GetName(), "mainGroup"))
		{
			Noesis::VisualState* state = group->GetCurrentState(_visualStateRoot);
			if (state && !std::strcmp(state->GetName().Str(), "InitialState"))
			{
				_appService->GetAppGlobals().Quit();
			}
			else
			{
				Noesis::VisualStateManager::GoToElementState(_visualStateRoot, Noesis::Symbol("InitialState"), true);
			}

			break;
		}
	}
}

void ReTron::TitlePageViewModel::OnTargetSizeChanged()
{
	OnPropertyChanged("FullScreenText");
}

NS_IMPLEMENT_REFLECTION(ReTron::TitlePage, "ReTron.TitlePage")
{
	NsProp("ViewModel", &ReTron::TitlePage::GetViewModel);
}

ReTron::TitlePage::TitlePage(IAppService* appService)
	: _appService(appService)
	, _viewModel(*new TitlePageViewModel(appService))
{
	Noesis::GUI::LoadComponent(this, "TitlePage.xaml");
	_viewModel->SetVisualStateRoot(FindName<Noesis::FrameworkElement>("rootPanel"));
}

ReTron::TitlePage::~TitlePage()
{
}

ReTron::TitlePageViewModel* ReTron::TitlePage::GetViewModel() const
{
	return _viewModel;
}
