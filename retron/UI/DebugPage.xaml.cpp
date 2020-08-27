#include "pch.h"
#include "Core/AppService.h"
#include "States/DebugState.h"
#include "States/GameState.h"
#include "UI/DebugPage.xaml.h"
#include "UI/Utility/DelegateCommand.h"

NS_IMPLEMENT_REFLECTION(ReTron::DebugPageViewModel, "ReTron.DebugPageViewModel")
{
	NsProp("RestartLevelCommand", &ReTron::DebugPageViewModel::_restartLevelCommand);
	NsProp("RestartGameCommand", &ReTron::DebugPageViewModel::_restartGameCommand);
	NsProp("RebuildResourcesCommand", &ReTron::DebugPageViewModel::_rebuildResourcesCommand);
	NsProp("CloseDebugCommand", &ReTron::DebugPageViewModel::_closeDebugCommand);
}

ReTron::DebugPageViewModel::DebugPageViewModel(IAppService* appService, DebugState* debugState)
	: _appService(appService)
	, _debugState(debugState)
	, _restartLevelCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::RestartLevel)))
	, _restartGameCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::RestartGame)))
	, _rebuildResourcesCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::RebuildResources)))
	, _closeDebugCommand(Noesis::MakePtr<ff::DelegateCommand>(Noesis::MakeDelegate(this, &ReTron::DebugPageViewModel::CloseDebug)))
{
}

void ReTron::DebugPageViewModel::RestartLevel(Noesis::BaseComponent* param)
{
	_debugState->_restartLevelEvent.publish();
}

void ReTron::DebugPageViewModel::RestartGame(Noesis::BaseComponent* param)
{
	_debugState->_restartGameEvent.publish();
}

void ReTron::DebugPageViewModel::RebuildResources(Noesis::BaseComponent* param)
{
	_debugState->_rebuildResourcesEvent.publish();
}

void ReTron::DebugPageViewModel::CloseDebug(Noesis::BaseComponent* param)
{
	_debugState->SetVisible(false);
}

NS_IMPLEMENT_REFLECTION(ReTron::DebugPage, "ReTron.DebugPage")
{
	NsProp("ViewModel", &ReTron::DebugPage::GetViewModel);
}

ReTron::DebugPage::DebugPage(IAppService* appService, DebugState* debugState)
	: _appService(appService)
	, _viewModel(*new DebugPageViewModel(appService, debugState))
{
	Noesis::GUI::LoadComponent(this, "DebugPage.xaml");
}

ReTron::DebugPageViewModel* ReTron::DebugPage::GetViewModel() const
{
	return _viewModel;
}
