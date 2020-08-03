#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Services/AppService.h"
#include "States/DebugState.h"
#include "UI/DebugPage.xaml.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

ReTron::DebugState::DebugState(IAppService* appService)
	: _appService(appService)
{
	_debugPage = *new ReTron::DebugPage(_appService, this);

	std::shared_ptr<ff::XamlView> view = _appService->GetXamlGlobals().CreateView(_debugPage);
	_viewState = std::make_shared<ff::XamlViewState>(view, _appService->GetTempTarget(TempTargets::RgbPma2), _appService->GetTempDepth(TempTargets::RgbPma2));
}

ReTron::DebugState::~DebugState()
{
}

bool ReTron::DebugState::GetVisible() const
{
	return _underState != nullptr;
}

void ReTron::DebugState::SetVisible(const std::shared_ptr<ff::State>& underState)
{
	_underState = underState;
}

void ReTron::DebugState::Hide()
{
	_underState = nullptr;
}

void ReTron::DebugState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	if (GetVisible())
	{
		_underState->Render(globals, target, depth);
		_appService->ClearTempTargets(TempTargets::RgbPma2);

		ff::State::Render(globals, target, depth);

		_appService->RenderTempTargets(TempTargets::RgbPma2, target);
	}
}

size_t ReTron::DebugState::GetChildStateCount()
{
	return GetVisible() ? 1 : 0;
}

ff::State* ReTron::DebugState::GetChildState(size_t index)
{
	return _viewState.get();
}
