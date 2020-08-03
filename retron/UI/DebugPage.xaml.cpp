#include "pch.h"
#include "Services/AppService.h"
#include "States/GameState.h"
#include "UI/DebugPage.xaml.h"
#include "UI/Utility/DelegateCommand.h"

NS_IMPLEMENT_REFLECTION(ReTron::DebugPageViewModel, "ReTron.DebugPageViewModel")
{
	NsProp("RestartLevelCommand", &ReTron::DebugPageViewModel::_restartLevelCommand);
	NsProp("RestartGameCommand", &ReTron::DebugPageViewModel::_restartGameCommand);
	NsProp("RestartAppCommand", &ReTron::DebugPageViewModel::_restartAppCommand);
	NsProp("RebuildResourcesCommand", &ReTron::DebugPageViewModel::_rebuildResourcesCommand);
}

ReTron::DebugPageViewModel::DebugPageViewModel(IAppService* appService)
	: _appService(appService)
	, _restartLevelCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::RestartLevel)))
	, _restartGameCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::RestartGame)))
	, _restartAppCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::RestartApp)))
	, _rebuildResourcesCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::RebuildResources)))
{
}

void ReTron::DebugPageViewModel::RestartLevel(Noesis::BaseComponent* param)
{
	RestartLevelEvent.Notify();
}

void ReTron::DebugPageViewModel::RestartGame(Noesis::BaseComponent* param)
{
	RestartGameEvent.Notify();
}

void ReTron::DebugPageViewModel::RestartApp(Noesis::BaseComponent* param)
{
	RestartAppEvent.Notify();
}

void ReTron::DebugPageViewModel::RebuildResources(Noesis::BaseComponent* param)
{
	RebuildResourcesEvent.Notify();
}

NS_IMPLEMENT_REFLECTION(ReTron::DebugPage, "ReTron.DebugPage")
{
	NsProp("ViewModel", &ReTron::DebugPage::GetViewModel);
}

ReTron::DebugPage::DebugPage(IAppService* appService)
	: _appService(appService)
	, _viewModel(*new DebugPageViewModel(appService))
{
	Noesis::GUI::LoadComponent(this, "DebugPage.xaml");
}

ReTron::DebugPageViewModel* ReTron::DebugPage::GetViewModel() const
{
	return _viewModel;
}
